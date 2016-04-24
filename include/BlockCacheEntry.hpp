#ifndef BLOCKCACHEENTRY_HPP
# define BLOCKCACHEENTRY_HPP

# include <assert.h>

# include <Globals.hpp>
# include <LBA.hpp>

# include <Transaction.hpp>

class BlockCacheEntry
{
 friend class BlockCache;
 friend class Transaction;
 friend class BlockDevice;

 public:
  inline void
  setAsLeader(void)
  {
   assert(!allocated);
   assert(dirty);
   assert(!leader);
   
   leader = true;
  }

  /*! \todo destroy the BlockCacheEntry pointer too. */
  inline void
  unlock(register uint8_t* &               dataPointer,
         register class BlockCacheEntry* & entryPointer,
         register class Transaction* const transaction)
  {
   assert(locked);

   if (allocated)
   {
    if (!this->transaction)
    {  
     assert(!next);
     next = transaction->addToTransaction(this);
     this->transaction = transaction;
    }
   }

   locked--;
   dataPointer  = 0;
   entryPointer = 0;
  }

  uint8_t*
  getDataPointer(void)
  {
   assert(locked);
   
   return data;
  }

  /* Not inlined. In BlockCacheEntry.cpp */
  bool
  setLBA(register class VirtualBlockDevice* device,
         register const struct LBA          lba);

 private:
  static const unsigned int
  maxLocations = 3;

  bool allocated;
  bool accessed;
  bool dirty;
  bool leader;

  struct
  {
   struct LBA                lba;
   class VirtualBlockDevice* device;
   bool                      valid;
   bool                      transactional;
  } locations[maxLocations];
  
  uint32_t                         locked;

  BlockCacheEntry*                 next;
  const Transaction*               transaction;
  
  /*! \todo this should be in a separate array. */
  uint8_t
  data[sectorSize] __attribute__ ((aligned (32)));

  inline
  BlockCacheEntry()
  {
   allocated   = false;
   accessed    = false;
   dirty       = false;
   leader      = false;

   for(register unsigned int i = 0; i < maxLocations; i++)
    locations[i].valid = false;
    
   locked      = 0;
   next        = 0;
   transaction = 0;
  }

  uint8_t*
  getDataPointerUnsafe(void)
  {
   return data;
  }

  inline void
  setDirty(void)
  {
   assert(locked);
   
   dirty = true;
  }

  inline void
  lock(void)
  {
   assert(locked < UINT32_MAX);
   
   locked++;
  }
};

#endif
