#ifndef BPLUSTREE_HPP
# define BPLUSTREE_HPP

# include <assert.h>
# include <stdint.h>
# include <string.h>

# include <Globals.hpp>
# include <Key.hpp>
# include <LBA.hpp>
# include <BlockCacheEntry.hpp>
# include <BlockCache.hpp>
# include <FileSystem.hpp>
# include <Transaction.hpp>
# include <Serializable.hpp>

# include <RawData.hpp>

class BPlusTree
{
 friend class FileSystem;
  
 public:
  enum BPlusTreeError
  {
   noError = 0,
   keyNotFound,
   dataTooBig,
   sizeIsNotAcceptable
  };

  inline bool
  lookup(register Serializable&            destination,
         register uint_fast16_t&           size,
         register enum BPlusTreeError&     error,
         register const Key                key,
         register class Transaction* const transaction) const
  {
   register BlockCacheEntry* cacheEntry;
   register enum BlockCache::BlockCacheError cacheError;

   if (!BlockCache::getInstance().readLookup(cacheEntry, cacheError, transaction, rootDevice, rootLBA))
   {
    assert(0);
   }

   assert(cacheEntry);
   
   return lookup(destination, size, error, cacheEntry, key, transaction);
  }

  inline bool
  insert(register const BPlusTree* &       newTree,
         register enum BPlusTreeError&     error,
         register const Serializable&      source,
         register const Key                key,
         register class Transaction* const transaction) const
  {
   register BlockCacheEntry* cacheEntry;
   register enum BlockCache::BlockCacheError cacheError;

   assert(transaction);

   if (!BlockCache::getInstance().readLookup(cacheEntry, cacheError, transaction, rootDevice, rootLBA))
   {
    assert(0);
   }

   assert(cacheEntry);

   register struct LBA newLBA;
   
   if (!insertOrRemove(newLBA, error, cacheEntry, source, key, transaction, false))
   {
    assert(0);
   }

   assert(error == noError);

   newTree = new BPlusTree(rootDevice, newLBA);
   
   assert(newTree);

   error = noError;
   return true;
  }

  inline bool
  remove(register const BPlusTree* &       newTree,
         register enum BPlusTreeError&     error,
         register const Key                key,
         register class Transaction* const transaction) const
  {
   register BlockCacheEntry* cacheEntry;
   register enum BlockCache::BlockCacheError cacheError;

   assert(transaction);

   if (!BlockCache::getInstance().readLookup(cacheEntry, cacheError, transaction, rootDevice, rootLBA))
   {
    assert(0);
   }

   assert(cacheEntry);
   
   register struct LBA newLBA;
   register class RawData dummy;   
   
   if (insertOrRemove(newLBA, error, cacheEntry, dummy, key, transaction, true))
   {
    assert(0);
   }

   assert(error == noError);

   newTree = new BPlusTree(rootDevice, newLBA);
   
   assert(newTree);

   error = noError;
   return true;
  }

 private:
  struct header
  {
   uint8_t  version;
   uint8_t  keys;
   uint16_t spaceUsedNFlags;
  };

  static const uint16_t isRoot = 0x8000;
  static const uint16_t isLeaf = 0x4000;
  
  struct key
  {
   uint64_t major;
   uint64_t minor;
   uint16_t offset;
   uint16_t size;
   uint8_t  type;
   uint8_t  isLocation;
  };

  struct firstLocation
  {
   uint16_t offset;
  };
  
  struct location
  {
   uint64_t theLBA;
   uint16_t size;
  };

  static const uint8_t
  version = 0x7f;

  static const uint8_t
  bigEndian = 0x80;

  class VirtualBlockDevice* rootDevice;
  LBA                       rootLBA;

  /*! Build a new root. This is only used for some initial tests. The
      constructor will be removed. */
  BPlusTree(register class VirtualBlockDevice* const device,
            register class FileSystem* const         fileSystem,
            register class Transaction* const        transaction);

  /* Actual constructor for most situations. */
  inline
  BPlusTree(register class VirtualBlockDevice* const device,
           register const struct LBA                theLBA)
  {
   rootDevice = device;
   rootLBA    = theLBA;
  }
  
  static inline bool
  equalKey(register const struct key* const keyStruct,
           register const Key               key,
           register const bool              isLittle)
  { 
   return ((key.type == keyStruct->type) &&
           (key.major == fromFileSystemEndian(&(keyStruct->major), isLittle)) &&
           (key.minor == fromFileSystemEndian(&(keyStruct->minor), isLittle)));
  }

  static inline bool
  higherKey(register const struct key* const keyStruct,
            register const Key               key,
            register const bool              isLittle)
  {
   register bool higher = key.type  > keyStruct->type;

   if (!higher && (key.type == keyStruct->type))
   {
    higher = key.major > fromFileSystemEndian(&(keyStruct->major), isLittle);

    if (!higher && (key.major == fromFileSystemEndian(&(keyStruct->major), isLittle)))
      higher = key.minor > fromFileSystemEndian(&(keyStruct->minor), isLittle);
   }

   return higher;
  }
  
  static inline bool
  search(register unsigned int&           returnedKeyIndex,
         register const struct key* const keyArray,         
         register const unsigned int      keys,
         register const bool              isLeaf,
         register const bool              isLittle,
         register const Key               key)
  {
   returnedKeyIndex = 0;
    
   if (keys == 0)
    return false; 

   register unsigned int left  = 1;
   register unsigned int right = keys;

   while(left < right)
   {
    register const unsigned int middle = left + (right-left)/2;

    returnedKeyIndex = middle;

    if (middle == 0)
    {
     return !isLeaf;
    }
    
    if (equalKey(&keyArray[middle-1], key, isLittle))
     return true;

    
    if (higherKey(&keyArray[middle-1], key, isLittle))
    {
     left = middle + 1;
    }
    else
    {
     right = middle - 1;
    }
   }

   returnedKeyIndex = left;
   
   if (!isLeaf)
    return true;  
   
   if ((left > 0) &&
       equalKey(&keyArray[left-1], key, isLittle))
    return true;
       
   return false;
  }
  
  static inline bool
  lookup(register Serializable&            destination,
         register uint_fast16_t&           size,
         register enum BPlusTreeError&     error,
         register BlockCacheEntry*         cacheEntry,
         register const Key                key,
         register class Transaction* const transaction)
  {
   register uint8_t* data = cacheEntry->getDataPointer();
   register bool found = false;
   
   assert(data);

   if ((((struct header*)data)->version & ~0x80) != version)
   {
    /* unsupported version. */
    assert(0);
   }

   register const bool    isLittle =(((struct header*)data)->version & 0x80) != bigEndian;
   register const uint8_t keys     =((struct header*)data)->keys;
   register const bool    isLeaf   =
     (fromFileSystemEndian(&(((struct header*)data)->spaceUsedNFlags), isLittle) &
      BPlusTree::isLeaf) != 0; 

   register unsigned int keyIndex;
   found = search(keyIndex,
                 (const struct key*) (data + sizeof(struct header) +
                                             (isLeaf ? 0 :  sizeof(struct firstLocation))),
                 keys, isLeaf, isLittle, key);

   if (!found)
   {
    while(!isLeaf)
     assert(0);

    error = keyNotFound;
   }
   else 
   {
    if (isLeaf)
    {
     assert(keyIndex > 0);
     assert(keyIndex <= keys);
     register const struct key* const key = ((const struct key*) (data + sizeof(struct header))) + keyIndex - 1;

     register const uint16_t dataSize = fromFileSystemEndian(&(key->size), isLittle);

     size  = dataSize;

     if (destination.size() < dataSize)
     {
      error = dataTooBig;
      found = false;
     }
     else if (!destination.isSizeAcceptable(dataSize))
     {
      error = sizeIsNotAcceptable;
      found = false;
     }
     else
     {
      register const uint16_t offset = fromFileSystemEndian(&(key->offset), isLittle);

      if (key->isLocation)
      {
       /* Data in sector. */ 
       assert(0);
      }
      else
      {
       assert(offset <= sectorSize);
       assert((offset + dataSize) <= sectorSize);

       if (!destination.fromFileSystem(data+offset, dataSize, isLittle))
        assert(0);
      }
      
      error = noError;
     }
    }
   }
   
   cacheEntry->unlock(data, cacheEntry, transaction);

   if (isLeaf)
    return found; 
  
   /* Now descent. */
   
   assert(0);
  }
  
  inline bool
  insertOrRemove(register struct LBA &             newLBA,
                 register enum BPlusTreeError&     error,
                 register BlockCacheEntry*         cacheEntry,
                 register const Serializable&      source,
                 register const Key                key,
                 register class Transaction* const transaction,
                 register const bool               remove) const
  {
   uint8_t* data = cacheEntry->getDataPointer();

   assert(data);

   if ((((struct header*)data)->version & ~0x80) != version)
   {
    /* unsupported version. */
    assert(0);
   }

   register const bool    isLittle =(((struct header*)data)->version & 0x80) != bigEndian;
   register const uint8_t keys = ((struct header*)data)->keys;
   register const bool    isLeaf   =
    (fromFileSystemEndian(&(((struct header*)data)->spaceUsedNFlags), isLittle) &
     BPlusTree::isLeaf) != 0; 

   register unsigned int keyIndex;
   register bool found = search(keyIndex,
                                (const struct key*) (data + sizeof(struct header) +
                                                          (isLeaf ? 0 :  sizeof(struct firstLocation))),
                                keys, isLeaf, isLittle, key);

   register bool success = false;
   
   if (!isLeaf)
   {
    /* Need to descend. */
    assert(0);
   }
   else
   {
    if (remove && !found)
    {
     /* Cannot remove since the key is not found. */ 
     error = keyNotFound;
    }
    else
    {
     /* Add, replace or remove key or value. */
     assert(keyIndex <= keys);

     register uint16_t     dataSpace = fromFileSystemEndian(&(((struct header*)data)->spaceUsedNFlags),
                                                            isLittle) & (sectorSize-1);
     register unsigned int usedSpace = sizeof(struct header) +
                                        (isLeaf ? 0 :  sizeof(struct firstLocation)) +
                                        keys * sizeof(struct key) +
                                        dataSpace;

     register uint16_t     existingSize = 0;

     assert(usedSpace < sectorSize);
     
     if (found)
     {
      assert(keyIndex > 0);
      register const struct key* const key = ((const struct key*) (data + sizeof(struct header))) + keyIndex - 1;
      existingSize = fromFileSystemEndian(&(key->size), isLittle);
     }
     
     if (!remove)
     {
      if (usedSpace + 2*sizeof(struct key) + source.size() - existingSize > sectorSize)
       /* Need to split node or place data in a new sector. */
       assert(0);
     }

     /* Allocate new cache entry and copy data as we traverse the key array. */
     register class BlockCacheEntry*           newCacheEntry;
     register enum BlockCache::BlockCacheError cacheError;

     if (!BlockCache::getInstance().allocate(newCacheEntry, cacheError, transaction))
     {
      assert(0);
     }

     assert(cacheError == BlockCache::noError);
   
     register uint8_t* newNodeData = newCacheEntry->getDataPointer();

     assert(newNodeData);

     /* Clear the node. */
     memset(newNodeData, 0, sectorSize);

     register uint16_t newSize = 0;

     register const struct key* oldKeys = ((const struct key*) (data + sizeof(struct header)));
     register struct key*       newKeys = ((struct key*) (newNodeData + sizeof(struct header)));

     register uint8_t newNbrOfKeys = keys;
     
     if (keyIndex == 0)
      keyIndex = 1;

     if (keys == 0)
     {
      /* Bootstrap an empty node. */
      newNbrOfKeys = 1;

      toFileSystemEndian(&newKeys->major, key.major, isLittle);
      toFileSystemEndian(&newKeys->minor, key.minor, isLittle);
      newKeys->type = key.type;
      newKeys->isLocation = 0;

      assert(source.size() >= 0);
      assert(source.size() < sectorSize);

      toFileSystemEndian(&newKeys->size, source.size(), isLittle);

      newSize += source.size();

      register const uint16_t offset = sectorSize - newSize;

      assert(offset < sectorSize);
      
      toFileSystemEndian(&newKeys->offset, offset, isLittle);

      if(!source.toFileSystem(newNodeData + offset, isLittle))
       assert(0);
     }
     else
     {  
      for(register unsigned int index = 1; index <= keys; index++, oldKeys++)
      {
       if (index == keyIndex)
       {
        if (remove)
        {
         assert(newNbrOfKeys > 0);
         newNbrOfKeys--;
         /* We have earlier tested for the error case: remove && !found. */
         continue;
        }

        newNbrOfKeys++;

        toFileSystemEndian(&newKeys->major, key.major, isLittle);
        toFileSystemEndian(&newKeys->minor, key.minor, isLittle);
        newKeys->type = key.type;
        newKeys->isLocation = 0;

        assert(source.size() >= 0);
        assert(source.size() < sectorSize);

        toFileSystemEndian(&newKeys->size, source.size(), isLittle);

        newSize += source.size();

        register const uint16_t offset = sectorSize - newSize;

        assert(offset < sectorSize);
      
        toFileSystemEndian(&newKeys->offset, offset, isLittle);

        if (!source.toFileSystem(newNodeData + offset, isLittle))
         assert(0);

        newKeys++;	 
       
        if (found)
         /* replace the existing <key, value> */ 
         continue;
       }
      
       *newKeys = *oldKeys;

       register const uint16_t oldOffset = fromFileSystemEndian(&oldKeys->offset, isLittle);
       register const uint16_t oldSize = fromFileSystemEndian(&oldKeys->size, isLittle);

       assert(oldSize >= 0);
       assert(oldSize < sectorSize);
       assert(oldOffset < sectorSize);

       newSize += oldSize;
       
       register const uint16_t offset = sectorSize - newSize;

       assert(offset < sectorSize);
       
       toFileSystemEndian(&newKeys->offset, offset, isLittle);

       memcpy(newNodeData + offset, data + oldOffset, oldSize);

       newKeys++;
      }
     }

     assert(((uintptr_t)newKeys) < (((uintptr_t)newNodeData) + sectorSize - newSize));
     
     register struct header* const newHeader = (struct header*) newNodeData;

     assert(newNbrOfKeys <= ((sectorSize - sizeof(struct header) - newSize) / sizeof(struct key)));

     newHeader->version = version | (isLittle ? 0 : bigEndian);
     newHeader->keys    = newNbrOfKeys;
     /*! \todo transfer root flag from old entry. */
     toFileSystemEndian(&newHeader->spaceUsedNFlags,
                        newSize | BPlusTree::isRoot | BPlusTree::isLeaf, isLittle);

     register enum FileSystem::FileSystemError fileSystemError;

     if (!transaction->getFileSystem()->getAvailableLBA(newLBA, fileSystemError))
      assert(0); 

     assert(fileSystemError == FileSystem::noError);
     
     if (!newCacheEntry->setLBA(rootDevice, newLBA))
     {
      assert(0);
     }
    
     newCacheEntry->unlock(newNodeData, newCacheEntry, transaction);

     error = noError;
     success = true;
    }
   }
   
   cacheEntry->unlock(data, cacheEntry, transaction);
   return success;
  }
};

#endif
