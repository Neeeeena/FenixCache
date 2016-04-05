/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef BLOCKCACHE_HPP
# define BLOCKCACHE_HPP

# include <assert.h>

# include <LBA.hpp>
# include <BlockCacheEntry.hpp>
# include <VirtualBlockDevice.hpp>

class BlockCache
{
 public:
  enum BlockCacheError
  {
   noError = 0
  };

  static inline BlockCache& 
  getInstance()
  {
   return instance;
  }

  inline bool
  readLookup(register BlockCacheEntry* &              returnedCacheEntry,
             register enum BlockCacheError&           error,
             register const class Transaction* const  transaction,
             register class VirtualBlockDevice* const device,
             register const struct LBA                theLBA)
  {
   return lookup(returnedCacheEntry, error, transaction, device, theLBA, false);
  }
   

  inline bool
  readWriteLookup(register BlockCacheEntry* &              returnedCacheEntry,
                  register enum BlockCacheError&           error,
                  register const class Transaction* const  transaction,
                  register class VirtualBlockDevice* const device,
                  register const struct LBA                theLBA)
  {
   return lookup(returnedCacheEntry, error, transaction, device, theLBA, true);
  }

  inline bool
  allocate(register BlockCacheEntry* &             returnedCacheEntry,
           register enum BlockCacheError&          error,
           register const class Transaction* const transaction)
  {
   register unsigned int index = findEntry();

   assert(index < cacheEntries);

   entries[index].locked    = 0;
   entries[index].lock();
   entries[index].allocated = true;
   entries[index].accessed  = true;
   entries[index].setDirty();
   entries[index].leader  = false;

   returnedCacheEntry = &entries[index];
   error = noError;
   return true;
  }

  inline void
  addToHashTable(register BlockCacheEntry* const cacheEntry,
                 register unsigned int location)
  {
   register unsigned int hashIndex = calculateHashIndex(cacheEntry->locations[location].device,
                                                        cacheEntry->locations[location].lba); 
   
   assert(cacheEntry->locations[location].valid);
   assert((((uintptr_t) cacheEntry) & 3) == 0);
   assert(hashIndex < (BlockCacheEntry::maxLocations * cacheEntries));

   for(;
       hashBuckets[hashIndex] && ((uintptr_t)hashBuckets[hashIndex] != 0x1);
       hashIndex = (hashIndex + 1) % (BlockCacheEntry::maxLocations * cacheEntries));

   assert(!hashBuckets[hashIndex] || ((uintptr_t)hashBuckets[hashIndex] == 0x1));
   hashBuckets[hashIndex] = cacheEntry;
  }

  inline void
  removeFromHashTable(register const BlockCacheEntry* const cacheEntry,
                      register unsigned int location)
  {
   register unsigned int hashIndex = calculateHashIndex(cacheEntry->locations[location].device,
                                                        cacheEntry->locations[location].lba); 
   
   assert(cacheEntry->locations[location].valid);
   assert((((uintptr_t) cacheEntry) & 3) == 0);
   assert(hashIndex < (BlockCacheEntry::maxLocations * cacheEntries));

   for(;
       hashBuckets[hashIndex] && (hashBuckets[hashIndex] != cacheEntry);
       hashIndex = (hashIndex + 1) % (BlockCacheEntry::maxLocations * cacheEntries));

   assert(hashBuckets[hashIndex] == cacheEntry);
   /* Place tombstone. */
   hashBuckets[hashIndex] = (BlockCacheEntry*) 0x1;
  }

 private: 
  static const unsigned int
  cacheEntries = 16 * 1024;

  static BlockCache
  instance;

  BlockCacheEntry
  entries[cacheEntries];

  BlockCacheEntry*
  hashBuckets[BlockCacheEntry::maxLocations * cacheEntries];

  unsigned int
  clockIndex;
  
  inline
  BlockCache(void)
  {
   clockIndex = 0;
    
   for(register unsigned int i = 0; i < BlockCacheEntry::maxLocations * cacheEntries; i++)
    hashBuckets[i] = 0;   
  }

  inline unsigned int
  findEntry(void)
  {
   bool          done = false;
   unsigned int returnValue;
   
   /*! \todo rewrite into not using fields directly. */
   for(; !done; clockIndex = (clockIndex + 1) % cacheEntries)
   {
    if (entries[clockIndex].locked)
     continue;

    if (entries[clockIndex].leader)
     continue;    

    if (entries[clockIndex].accessed)
    {
     entries[clockIndex].accessed = false;
     continue;
    }

    if (entries[clockIndex].dirty)
    {
     /*! \todo handle the case when multiple locations are placed on an entry. */
     register bool alreadyWritten = false;
     
     for(register unsigned int location = 0; location < BlockCacheEntry::maxLocations; location++)
     {
      if (entries[clockIndex].locations[location].transactional)
      {
       assert(!alreadyWritten);
       assert(0);
      }
      
      if (entries[clockIndex].locations[location].valid)
      {
       assert(!alreadyWritten);

       register enum VirtualBlockDevice::VirtualBlockDeviceError blockError;

       assert(entries[clockIndex].locations[location].device);
       
       if (!entries[clockIndex].locations[location].device->
             writeSector(blockError, &entries[clockIndex], entries[clockIndex].locations[location].lba))
       {
        assert(0);
       }

       assert(blockError == VirtualBlockDevice::noError);
       
       alreadyWritten = true;
      }
     }
       
     entries[clockIndex].dirty = false;
     continue;
    }

    for(register unsigned int location = 0; location < BlockCacheEntry::maxLocations; location++)
    {
     if (entries[clockIndex].locations[location].valid)
      removeFromHashTable(&entries[clockIndex], location);
      
     entries[clockIndex].locations[location].valid = false;
    }
    
    done = true;
    returnValue = clockIndex;
   }

   return returnValue;
  }

  inline unsigned int
  calculateHashIndex(register const class VirtualBlockDevice* const device,
                     register const struct LBA                      lba)
  {
   register const unsigned int hashIndex =
    (((uintptr_t)device) ^
     (((uintptr_t)device) >> 32) ^
     lba.theLBA ^
     lba.theLBA >> 32) %
     (BlockCacheEntry::maxLocations * cacheEntries);
   assert(hashIndex < (BlockCacheEntry::maxLocations * cacheEntries));

   return hashIndex;
  }

  inline bool
  lookup(register BlockCacheEntry* &              returnedCacheEntry,
         register enum BlockCacheError&           error,
         register const class Transaction* const  transaction,
         register class VirtualBlockDevice* const device,
         register const struct LBA                theLBA,
         register const bool                      write)
  {
   register unsigned int hashIndex = calculateHashIndex(device,
                                                        theLBA); 
   
   assert(hashIndex < (BlockCacheEntry::maxLocations * cacheEntries));

   for(;
       hashBuckets[hashIndex];
       hashIndex = (hashIndex + 1) % (BlockCacheEntry::maxLocations * cacheEntries))
   {
    for(register unsigned int location = 0;
        location < BlockCacheEntry::maxLocations;
        location++)
    {
     if (hashBuckets[hashIndex]->locations[location].valid &&
         (hashBuckets[hashIndex]->locations[location].device == device) &&
         (hashBuckets[hashIndex]->locations[location].lba.theLBA == theLBA.theLBA))
      goto done;
    }
   }

done:
   if (hashBuckets[hashIndex])
   {
    /*! \todo add readers-writer locking. */
    assert(!hashBuckets[hashIndex]->locked);

    if (hashBuckets[hashIndex]->allocated &&
        (hashBuckets[hashIndex]->transaction != transaction))
     assert(0);
   
    hashBuckets[hashIndex]->lock();

    if (write)
     hashBuckets[hashIndex]->setDirty();

    hashBuckets[hashIndex]->accessed = true;
   
    returnedCacheEntry = hashBuckets[hashIndex];
    error = noError;
    return true;
   }

   /* Load the sector. */
   register unsigned int index = findEntry();
   
   assert(index < cacheEntries);

   entries[index].locked    = 0;
   entries[index].lock();
   entries[index].allocated = false;
   entries[index].accessed  = true;
   entries[index].dirty     = false;

   if (write)
    entries[index].setDirty();

   entries[index].leader    = false;
   
   register enum VirtualBlockDevice::VirtualBlockDeviceError blockError;

   assert(device);
       
   if (!device->readSector(blockError, &entries[index], theLBA))
   {
    assert(0);
   }

   assert(blockError == VirtualBlockDevice::noError);

   entries[index].setLBA(device, theLBA);
   
   returnedCacheEntry = &entries[index];
   error = noError;
   return true;
  }
};

#endif
