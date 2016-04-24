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
   register BlockCacheEntry*                 cacheEntry;
   register enum BlockCache::BlockCacheError cacheError;

   assert(transaction);

   if (!BlockCache::getInstance().readLookup(cacheEntry, cacheError, transaction, rootDevice, rootLBA))
   {
    assert(0);
   }

   assert(cacheEntry);

   register const bool               isLittle = (((struct header*)(cacheEntry->getDataPointer()))->version & 0x80) != bigEndian;

   register struct splitAndMergeInfo childrenInfo;
   
   if (!insertOrRemove(childrenInfo, error, cacheEntry, source, key, transaction, false))
   {
    assert(0);
   }

   assert(error == noError);

   if (!createNewRoot(newTree, error, childrenInfo, transaction, isLittle))
   {
    assert(0);
   }

   assert(newTree);
   assert(error == noError);

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

   register const bool               isLittle = (((struct header*)(cacheEntry->getDataPointer()))->version & 0x80) != bigEndian;
   
   register struct splitAndMergeInfo childrenInfo;
   register class RawData            dummy;   
   
   if (!insertOrRemove(childrenInfo, error, cacheEntry, dummy, key, transaction, true))
   {
    assert(0);
   }

   assert(error == noError);

   if (!createNewRoot(newTree, error, childrenInfo, transaction, isLittle))
   {
    assert(0);
   }

   assert(newTree);
   assert(error == noError);

   error = noError;
   return true;
  }

 private:
  struct __attribute__ ((__packed__)) header
  {
   uint8_t  version;
   uint8_t  keys;
   uint16_t spaceUsedNFlags;
  };

  static const uint16_t isLeaf = 0x8000;
  
  struct __attribute__ ((__packed__)) leafKey
  {
   uint64_t major;
   uint64_t minor;
   uint16_t offset;
   uint16_t size;
   uint8_t  type;
   bool     isLocation:1;
  };

  struct __attribute__ ((__packed__)) internalKey
  {
   uint64_t major;
   uint64_t minor;
   uint16_t offset;
   uint8_t  type;
  };

  struct __attribute__ ((__packed__)) firstLocation
  {
   uint16_t offset;
  };
  
  struct __attribute__ ((__packed__)) leafLocation
  {
   uint64_t theLBA;
   uint16_t size;
  };

  struct __attribute__ ((__packed__)) internalLocation
  {
   uint64_t theLBA;
  };

  struct splitAndMergeInfo
  {
   struct
   {
    bool     valid;
    Key      key;
    LBA      theLBA;
    uint16_t size;
    uint8_t  keys;
    bool     leaf;
   } children[3];

   void inline
   init(void)
   {
    for(register unsigned int i = 0; i < 3; i++)
    {
     children[i].valid = false;
    }
   }
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
  equalInternalKey(register const struct internalKey* const keyStruct,
                   register const Key                       key,
                   register const bool                      isLittle)
  { 
   return ((key.type == keyStruct->type) &&
           (key.major == fromFileSystemEndian(&(keyStruct->major), isLittle)) &&
           (key.minor == fromFileSystemEndian(&(keyStruct->minor), isLittle)));
  }

  static inline bool
  equalLeafKey(register const struct leafKey* const keyStruct,
               register const Key                   key,
               register const bool                  isLittle)
  { 
   return ((key.type == keyStruct->type) &&
           (key.major == fromFileSystemEndian(&(keyStruct->major), isLittle)) &&
           (key.minor == fromFileSystemEndian(&(keyStruct->minor), isLittle)));
  }


  static inline bool
  higherInternalKey(register const struct internalKey* const keyStruct,
                    register const Key                       key,
                    register const bool                      isLittle)
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
  higherLeafKey(register const struct leafKey* const keyStruct,
                register const Key                   key,
                register const bool                  isLittle)
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
  search(register unsigned int&        returnedKeyIndex,
         register const uint8_t* const keyArray,         
         register const unsigned int   keys,
         register const bool           isLeaf,
         register const bool           isLittle,
         register const Key            key)
  {
   returnedKeyIndex = 0;
    
   if (keys == 0)
    return false; 

   register unsigned int left  = 1;
   register unsigned int right = keys;

   while(left <= right)
   {
    register const unsigned int middle = (left + right) / 2;

    returnedKeyIndex = middle;

    assert(middle > 0);
    
    if ((isLeaf && equalLeafKey((struct leafKey*)(keyArray + sizeof(struct leafKey) * (middle-1)), key, isLittle)) ||
        (!isLeaf && equalInternalKey((struct internalKey*)(keyArray + sizeof(struct internalKey) * (middle-1)), key, isLittle)))
     return true;

    
    if ((isLeaf && higherLeafKey((struct leafKey*)(keyArray + sizeof(struct leafKey) * (middle-1)), key, isLittle)) ||
        (!isLeaf && higherInternalKey((struct internalKey*)(keyArray + sizeof(struct internalKey) * (middle-1)), key, isLittle)))
    {
     left = middle + 1;
    }
    else
    {
     right = middle - 1;
    }
   }
 
   if ((isLeaf) &&
       (returnedKeyIndex > 0) &&
       equalLeafKey((struct leafKey*)(keyArray + sizeof(struct leafKey) * (returnedKeyIndex-1)), key, isLittle))
    return true;

   while ((returnedKeyIndex > 0) &&
          ((isLeaf && !higherLeafKey((struct leafKey*)(keyArray + sizeof(struct leafKey) * (returnedKeyIndex-1)), key, isLittle)) ||
          (!isLeaf && !higherInternalKey((struct internalKey*)(keyArray + sizeof(struct internalKey) * (returnedKeyIndex-1)), key, isLittle))))
   {
    returnedKeyIndex--;
   }
    
   return !isLeaf;
  }
  
  inline bool
  lookup(register Serializable&            destination,
         register uint_fast16_t&           size,
         register enum BPlusTreeError&     error,
         register BlockCacheEntry*         cacheEntry,
         register const Key                key,
         register class Transaction* const transaction) const
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

   register unsigned int  keyIndex;
   found = search(keyIndex,
                  data + sizeof(struct header) + (isLeaf ? 0 :  sizeof(struct firstLocation)),
                  keys, isLeaf, isLittle, key);

   register uint16_t      dataSize    = 0;
   register LBA           indirectLBA;
   register bool          indirect    = false;

   if (!found)
   {
    /* this should not be possible! */
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
     register const struct leafKey* const key = ((const struct leafKey*) (data + sizeof(struct header))) + keyIndex - 1;

     dataSize = fromFileSystemEndian(&(key->size), isLittle);
     register const uint16_t         offset = fromFileSystemEndian(&(key->offset), isLittle);

     if (key->isLocation)
     {
      assert(dataSize == sizeof(struct leafLocation));

      assert(offset > (sizeof(struct header) + sizeof(struct leafKey)));
      assert(offset < (sectorSize - sizeof(struct leafLocation)));  

      register const struct leafLocation* const location = ((const struct leafLocation*) (data + offset));
      
      dataSize = fromFileSystemEndian(&(location->size), isLittle);

      assert(dataSize <= sectorSize);

      indirectLBA.theLBA = fromFileSystemEndian(&(location->theLBA), isLittle);
      indirect = true;
     }

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
      if (!key->isLocation)
      {
       assert(offset <= sectorSize);
       assert((offset + dataSize) <= sectorSize);

       if (!destination.fromFileSystem(data+offset, dataSize, isLittle))
        assert(0);
      }
      
      error = noError;
     }
    }
    else
    {
     /* Extract a location to decend into. */
     assert(keyIndex >= 0);
     assert(keyIndex <= keys);

     register uint16_t         offset;

     if (keyIndex == 0)
     {
      register const struct firstLocation* const location = ((const struct firstLocation*) (data  + sizeof(struct header)));
      offset = fromFileSystemEndian(&(location->offset), isLittle);
     }
     else
     {
      register const struct internalKey* const key = ((const struct internalKey*) (data  + sizeof(struct header) + sizeof(struct firstLocation))) + keyIndex - 1;
      offset = fromFileSystemEndian(&(key->offset), isLittle);
     }

     assert(offset > (sizeof(struct header) + sizeof(struct firstLocation)));
     assert(offset <= (sectorSize - sizeof(struct internalLocation)));

     register const struct internalLocation* const location = (const struct internalLocation*) (data  + offset);
  
     indirectLBA.theLBA = fromFileSystemEndian(&(location->theLBA), isLittle);
     indirect = true;
    }
   }
   
   cacheEntry->unlock(data, cacheEntry, transaction);

   if (indirect)
   { 
    /* Retrieve data from disk. */
    register enum BlockCache::BlockCacheError cacheError;
 
    if (!BlockCache::getInstance().readLookup(cacheEntry, cacheError, transaction, rootDevice, indirectLBA))
    {
     assert(0);
    }

    assert(cacheEntry);
   }

   if (isLeaf)
   {
    if (indirect)
    {
     if (!destination.fromFileSystem(cacheEntry->getDataPointer(), dataSize, isLittle))
      assert(0);     
    }

    return found; 
   }

   /* Now descend. */
   return lookup(destination, size, error, cacheEntry, key, transaction);
  }
  
  inline bool
  insertOrRemove(register struct splitAndMergeInfo& siblingsInfo,
                 register enum BPlusTreeError&      error,
                 register BlockCacheEntry*          cacheEntry,
                 register const Serializable&       source,
                 register const Key                 key,
                 register class Transaction* const  transaction,
                 register const bool                remove) const
  {
   uint8_t* data = cacheEntry->getDataPointer();

   siblingsInfo.init();

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
                                data + sizeof(struct header) + (isLeaf ? 0 :  sizeof(struct firstLocation)),
                                keys, isLeaf, isLittle, key);

   register bool success = false;
   
   if (isLeaf)
   {
    if (remove && !found)
    {
     /* Cannot remove since the key is not found. */ 
     error = keyNotFound;
    }
    else
    {
     /* Add, replace or remove key or value. */

     register class LBA          dataLBA;

     assert(keyIndex <= keys);

     register const uint16_t     dataSpace =
      fromFileSystemEndian(&(((struct header*)data)->spaceUsedNFlags), isLittle) & (sectorSize-1);
     register const unsigned int usedSpace = sizeof(struct header) +
                                             keys * sizeof(struct leafKey) +
                                             dataSpace;

     register uint16_t           existingSize = 0;
     register uint16_t           targetSize = sectorSize;

     register bool               indirect = false;

     assert(usedSpace < sectorSize);
     
     if (found)
     {
      assert(keyIndex > 0);
      register const struct leafKey* const key = ((const struct leafKey*) (data + sizeof(struct header))) + keyIndex - 1;
      existingSize = fromFileSystemEndian(&(key->size), isLittle) + sizeof(struct leafKey);
     }
     
     register class BlockCacheEntry*           newCacheEntry;
     register enum BlockCache::BlockCacheError cacheError;

     if (!remove)
     {
      if (source.size() > (sectorSize - sizeof(struct header) - sizeof(struct leafKey)))
      {
       /* Need to store the data in a separate sector. */
       if (!BlockCache::getInstance().allocate(newCacheEntry, cacheError, transaction))
       {
        assert(0);
       }

       assert(cacheError == BlockCache::noError);
   
       register uint8_t* newNodeData = newCacheEntry->getDataPointer();

       assert(newNodeData);

       memset(newNodeData, 0, sectorSize);
       
       assert(source.size() >= 0);
       assert(source.size() < sectorSize);

       if(!source.toFileSystem(newNodeData, isLittle))
        assert(0);

       register enum FileSystem::FileSystemError fileSystemError;

       if (!transaction->getFileSystem()->getAvailableLBA(dataLBA, fileSystemError))
        assert(0); 

       assert(fileSystemError == FileSystem::noError);

       if (!newCacheEntry->setLBA(rootDevice, dataLBA))
       {
        assert(0);
       }

       newCacheEntry->unlock(newNodeData, newCacheEntry, transaction);

       indirect = true;
      }

      if ((usedSpace + sizeof(struct leafKey) + 
           (indirect ? sizeof(struct leafLocation) : source.size()) - existingSize) >= sectorSize)
      {
       /* Need to split node. In the worst case scenario we could end
        * up with three new sectors if the newly added value is very
        * large. */

       /* This is a little approxiate as we only consider the payload
        * size. Realistically, that is what make sense to use
        * though. */
       targetSize = (usedSpace + sizeof(struct leafKey) + 
                     (indirect ? sizeof(struct leafLocation) : source.size()) -
                     existingSize) / 2;
      }
     }

     register const struct leafKey* oldKeys = ((const struct leafKey*) (data + sizeof(struct header)));
 
     register unsigned int siblingIndex = 0;
     
     register unsigned int index = 0;

     register bool         dataToBeProcessed = true;
        
     do
     {
      assert(siblingIndex < 3);       

      /* Allocate new cache entry and copy data as we traverse the key array. */ 
      if (!BlockCache::getInstance().allocate(newCacheEntry, cacheError, transaction))
      {
       assert(0);
      }

      assert(cacheError == BlockCache::noError);
   
      register uint8_t*        newNodeData = newCacheEntry->getDataPointer();

      assert(newNodeData);

      register struct leafKey* newKeys = ((struct leafKey*) (newNodeData + sizeof(struct header)));

      memset(newNodeData, 0, sectorSize);

      register uint16_t newSize    = 0;
      register uint16_t actualSize = sizeof(struct header);

      register uint8_t newNbrOfKeys = 0;

      if (keys == 0)
      {
       /* Bootstrap an empty node. */
       newNbrOfKeys = 1;

       toFileSystemEndian(&newKeys->major, key.major, isLittle);
       toFileSystemEndian(&newKeys->minor, key.minor, isLittle);
       newKeys->type = key.type;
       newKeys->isLocation = indirect;

       siblingsInfo.children[siblingIndex].key = key;

       assert(!indirect && source.size() >= 0);
       assert(!indirect && source.size() < sectorSize);

       toFileSystemEndian(&newKeys->size, (indirect ? sizeof(struct leafLocation) : source.size()),
                          isLittle);

       newSize += (indirect ? sizeof(struct leafLocation) : source.size());

       register const uint16_t offset = sectorSize - newSize;

       assert(offset < sectorSize);
      
       toFileSystemEndian(&newKeys->offset, offset, isLittle);

       if (!indirect)
       {
        if(!source.toFileSystem(newNodeData + offset, isLittle))
         assert(0);
       }
       else
       {
        /* Construct leafLocation. */
        register struct leafLocation* const location = (struct leafLocation *) (newNodeData + offset); 
        toFileSystemEndian(&location->theLBA, dataLBA.theLBA, isLittle);
        toFileSystemEndian(&location->size, source.size(), isLittle);
       }

       /* Make sure the loop is terminated. */
       index = 1;
      }
      else
      {
       for(; index <= keys; index++)
       {
rerun:
        /* Start a new node if we are over the targetSize. */
        if (actualSize >= targetSize)
         break;

        if (dataToBeProcessed || (index != keyIndex))
        {
         if (index == keyIndex)
         {
          dataToBeProcessed = false;

          if (remove)
          {
           /* We have earlier tested for the error case: remove && !found. */
           oldKeys++;
           continue;
          }

          if (found)
          {
           /* replace the existing <key, value> */ 
           oldKeys++;
           goto rerun;
          }
         }

         if (index == 0)
         {
          if (index == keyIndex)
           goto rerun;

          continue;
         }

         if (newNbrOfKeys == 0)
         {
          Key splitKey;

          splitKey.major = fromFileSystemEndian(&oldKeys->major, isLittle);
          splitKey.minor = fromFileSystemEndian(&oldKeys->minor, isLittle);
          splitKey.type  = oldKeys->type;

          siblingsInfo.children[siblingIndex].key = splitKey;
         }

         newNbrOfKeys++;

         *newKeys = *oldKeys;

         register const uint16_t oldOffset = fromFileSystemEndian(&oldKeys->offset, isLittle);
         register const uint16_t oldSize = fromFileSystemEndian(&oldKeys->size, isLittle);

         assert(oldSize >= 0);
         assert(oldSize < sectorSize);
         assert(oldOffset < sectorSize); 

         newSize += oldSize;
       
         actualSize += sizeof(struct leafKey) + oldSize;

         register const uint16_t offset = sectorSize - newSize;

         assert(offset < sectorSize);
         assert((((uint8_t*)(newKeys + 1)) - newNodeData) <= offset);
       
         toFileSystemEndian(&newKeys->offset, offset, isLittle);

         memcpy(newNodeData + offset, data + oldOffset, oldSize);

         newKeys++;
         oldKeys++;

         /* A little ugly but allows us to split nodes after any key. */
         if (index == keyIndex)
          goto rerun;
        }
        else if (index == keyIndex)
        {
         /* Check if the new data can fit in the sector. If not, split immediately. */
         register const uint16_t tmpNewSize =
          newSize + (indirect ? sizeof(struct leafLocation) : source.size());

         if ((((uint8_t*)(newKeys + 1)) - newNodeData) >= (sectorSize - tmpNewSize))
          break;
        
         if (newNbrOfKeys == 0)
         {
          siblingsInfo.children[siblingIndex].key = key;
         }

         newNbrOfKeys++;

         toFileSystemEndian(&newKeys->major, key.major, isLittle);
         toFileSystemEndian(&newKeys->minor, key.minor, isLittle);
         newKeys->type = key.type;
         newKeys->isLocation = indirect;

         assert(!indirect && (source.size() >= 0));
         assert(!indirect && (source.size() < sectorSize));

         toFileSystemEndian(&newKeys->size, (indirect ? sizeof(struct leafLocation) : source.size()), 
                            isLittle);

         newSize = tmpNewSize; 
         actualSize += sizeof(struct leafKey) +
                       (indirect ? sizeof(struct leafLocation) : source.size());

         register const uint16_t offset = sectorSize - newSize;

         assert(offset < sectorSize);
      
         assert((((uint8_t*)(newKeys + 1)) - newNodeData) <= offset);

         toFileSystemEndian(&newKeys->offset, offset, isLittle);
 
         if (!indirect)
         {
          if (!source.toFileSystem(newNodeData + offset, isLittle))
           assert(0);
         }
         else
         {
          /* Construct leafLocation. */
          register struct leafLocation* const location = (struct leafLocation *) (newNodeData + offset);  
          toFileSystemEndian(&location->theLBA, dataLBA.theLBA, isLittle);
          toFileSystemEndian(&location->size, source.size(), isLittle);
         }
     
         newKeys++;	 
        }
       }
      }

      assert(((uintptr_t)newKeys) <= (((uintptr_t)newNodeData) + sectorSize - newSize));

      /* Make sure we discard empty nodes. */
      if (newNbrOfKeys > 0)
      {
       register struct header* const newHeader = (struct header*) newNodeData;

       assert(newNbrOfKeys <= ((sectorSize - sizeof(struct header) - newSize) / sizeof(struct leafKey)));

       newHeader->version = version | (isLittle ? 0 : bigEndian);

       assert(newNbrOfKeys >= 1);

       newHeader->keys    = newNbrOfKeys;
  
       toFileSystemEndian(&newHeader->spaceUsedNFlags,
                          newSize | BPlusTree::isLeaf, isLittle);

       siblingsInfo.children[siblingIndex].size = newSize;
       siblingsInfo.children[siblingIndex].keys = newNbrOfKeys;
       siblingsInfo.children[siblingIndex].leaf = isLeaf;

       register enum FileSystem::FileSystemError fileSystemError;

       if (!transaction->getFileSystem()->getAvailableLBA(siblingsInfo.children[siblingIndex].theLBA, fileSystemError))
        assert(0); 

       assert(fileSystemError == FileSystem::noError);
     
       if (!newCacheEntry->setLBA(rootDevice, siblingsInfo.children[siblingIndex].theLBA))
       {
        assert(0);
       }
    
       siblingsInfo.children[siblingIndex].valid = true;

       siblingIndex++;
      }
      newCacheEntry->unlock(newNodeData, newCacheEntry, transaction);
     } while (index <= keys);

     error = noError;
     success = true;
    }
    cacheEntry->unlock(data, cacheEntry, transaction);
    return success;
   }

   /* Need to descend. */
   register uint16_t offset;

   if (keyIndex == 0)
   {
    register const struct firstLocation* const location =
     ((const struct firstLocation*) (data  + sizeof(struct header)));
    offset = fromFileSystemEndian(&(location->offset), isLittle);
   }
   else
   {
    register const struct internalKey* const key =
     ((const struct internalKey*) (data  + sizeof(struct header) + sizeof(struct firstLocation))) + keyIndex - 1;
    offset = fromFileSystemEndian(&(key->offset), isLittle);
   }

   assert(offset > (sizeof(struct header) + sizeof(struct firstLocation)));
   assert(offset <= (sectorSize - sizeof(struct internalLocation)));

   register const struct internalLocation* const location =
    (const struct internalLocation*) (data  + offset);
  

   register LBA indirectLBA;
   indirectLBA.theLBA = fromFileSystemEndian(&(location->theLBA), isLittle);

   /* Retrieve data from disk. */
   register class BlockCacheEntry*           newCacheEntry;
   register enum BlockCache::BlockCacheError cacheError;
 
   if (!BlockCache::getInstance().readLookup(newCacheEntry, cacheError, transaction, rootDevice, indirectLBA))
   {
    assert(0);
   }

   assert(newCacheEntry);
   assert(cacheError == BlockCache::noError);

   register struct splitAndMergeInfo childrenInfo;

   success = insertOrRemove(childrenInfo, error, newCacheEntry, source, key, transaction, remove);

   /* Check if we need to split nodes. Internal nodes can be split into at most two sectors. */
   register const uint16_t     dataSpace =
    fromFileSystemEndian(&(((struct header*)data)->spaceUsedNFlags), isLittle) & (sectorSize-1);
   register const unsigned int usedSpace = sizeof(struct header) +
                                           sizeof(struct firstLocation) +
                                           (keys - 1) * sizeof(struct internalKey) +
                                           dataSpace;
   register uint16_t           targetSize = sectorSize;

   assert(usedSpace < sectorSize);

   register unsigned int       children = 0;
   for(register unsigned int i = 0; i < 3; i++)
    if (childrenInfo.children[i].valid)
     children++;

   if ((children > 0) &&
       ((usedSpace + (children -  1) * (sizeof(struct internalKey) + sizeof(struct internalLocation))) >
       sectorSize))
   {
    targetSize = (dataSpace + 
                  (children -  1) * (sizeof(struct internalKey) + sizeof(struct internalLocation))) / 2;
   }

   register unsigned int siblingIndex = 0;
     
   register unsigned int index = 0;

   register unsigned int childIndex = 0;

   /* Weave children in. */
   do
   {
    assert(siblingIndex < 3);       

    if (!BlockCache::getInstance().allocate(newCacheEntry, cacheError, transaction))
    {
     assert(0);
    }

    assert(cacheError == BlockCache::noError);
   
    register uint8_t*                    newNodeData   = newCacheEntry->getDataPointer();

    assert(newNodeData);

    memset(newNodeData, 0, sectorSize);

    register struct header* const        newHeader     = (struct header*) newNodeData;
    register struct firstLocation* const firstLocation = (struct firstLocation*) (newHeader + 1);
    register struct internalKey*         internalKey   = (struct internalKey*) (firstLocation + 1);

    register uint8_t                     newNbrOfKeys = 0;
    register uint16_t                    newSize = 0;

    assert(keys > 0);

    /*! \todo add support for merging of small nodes. */

    for(; index <= keys; index++)
    {
rerunInternal:
     if (newSize >= targetSize)
      break;
 
     register LBA newLBA = {.theLBA = 0};
     register Key newKey = {.type = 0, .major = 0, .minor = 0};

     /* We take some short cuts here. For example, we assume we can at
      * least three keys in a sector. */

     if (index == keyIndex)
     {
      /* Regardless of operation keyIndex is replaced with the children
       * even if that means removing a key. */
      if (childIndex < 3)
      {
       if (!childrenInfo.children[childIndex].valid)
       {
        childIndex++;
        goto rerunInternal;
       }
       else
       {
        /* Extract information from childInfo. */
        newLBA = childrenInfo.children[childIndex].theLBA;
        newKey = childrenInfo.children[childIndex].key;
       }

       childIndex++;
      }
      else
       continue;
     }
     else
     { 
      /* Extract data from the old location. */

      register uint16_t oldOffset = 0;
      if (index == 0)
      {
       register const struct firstLocation* const location =
        ((const struct firstLocation*) (data  + sizeof(struct header)));
       oldOffset = fromFileSystemEndian(&(location->offset), isLittle);
      }
      else
      {
       register const struct internalKey* const key =
       ((const struct internalKey*) (data  + sizeof(struct header) + sizeof(struct firstLocation))) + index - 1;
       oldOffset = fromFileSystemEndian(&(key->offset), isLittle);

       /* Read key. */
       newKey.type = key->type;
       newKey.major = fromFileSystemEndian(&key->major, isLittle); 
       newKey.minor = fromFileSystemEndian(&key->minor, isLittle); 
      }

      /* Read LBA from the payload. */
      register const struct internalLocation* const location =
       (const struct internalLocation*) (data  + oldOffset);
  
      newLBA.theLBA = fromFileSystemEndian(&(location->theLBA), isLittle);
     }

     /* insert data into new node. */
     newSize += sizeof(struct internalLocation);

     register struct internalLocation* internalLocation = (struct internalLocation*) (newNodeData + sectorSize - newSize);
     toFileSystemEndian(&internalLocation->theLBA, newLBA.theLBA, isLittle);

     if (newNbrOfKeys == 0)
     {
      toFileSystemEndian(&firstLocation->offset, sectorSize - newSize, isLittle);

      /* Prepare for the possible case of no actual keys which means
       * that the "0" index must be promoted. */
      siblingsInfo.children[siblingIndex].theLBA = newLBA;
      siblingsInfo.children[siblingIndex].size = 0;
      siblingsInfo.children[siblingIndex].keys = 0;
      siblingsInfo.children[siblingIndex].leaf = false;       
     }
     else
     {
      internalKey->type = newKey.type;
      toFileSystemEndian(&internalKey->major, newKey.major, isLittle); 
      toFileSystemEndian(&internalKey->minor, newKey.minor, isLittle); 

      toFileSystemEndian(&internalKey->offset, sectorSize - newSize, isLittle); 
      internalKey++;
     }
      
     newNbrOfKeys++;

     if ((index == keyIndex) && (childIndex < 3))
      goto rerunInternal;
    }

    assert(((uintptr_t)internalKey) <= (((uintptr_t)newNodeData) + sectorSize - newSize));

    if (newNbrOfKeys > 1)
    {
     assert(newNbrOfKeys <= ((sectorSize - sizeof(struct header) - sizeof(struct firstLocation) - newSize) / sizeof(struct internalKey)));
     newHeader->version = version | (isLittle ? 0 : bigEndian);
     assert(newNbrOfKeys >= 1);
     newHeader->keys    = newNbrOfKeys - 1;
     toFileSystemEndian(&newHeader->spaceUsedNFlags, newSize, isLittle);

     siblingsInfo.children[siblingIndex].size = newSize;
     siblingsInfo.children[siblingIndex].keys = newNbrOfKeys;
     siblingsInfo.children[siblingIndex].leaf = false;

     register enum FileSystem::FileSystemError fileSystemError;

     if (!transaction->getFileSystem()->getAvailableLBA(siblingsInfo.children[siblingIndex].theLBA, fileSystemError))
      assert(0); 

     assert(fileSystemError == FileSystem::noError);
    
     if (!newCacheEntry->setLBA(rootDevice, siblingsInfo.children[siblingIndex].theLBA))
     {
      assert(0);
     }
    
     siblingsInfo.children[siblingIndex].valid = true;

     siblingIndex++;
    }
    else
    {
     /* Promote the child up. */
     if (siblingIndex == 0)
      siblingsInfo.children[siblingIndex].valid = true;
    }

    newCacheEntry->unlock(newNodeData, newCacheEntry, transaction);
   } while (index <= keys);

   cacheEntry->unlock(data, cacheEntry, transaction);
   return success;
  }


  inline bool
  createNewRoot(register const BPlusTree*&              newTree,
                register enum BPlusTreeError&           error,
                register const struct splitAndMergeInfo childrenInfo,
                register class Transaction* const       transaction,
                register const bool                     isLittle) const
  {
   if (childrenInfo.children[0].valid &&
       !childrenInfo.children[1].valid &&
       !childrenInfo.children[2].valid)
   {
    newTree = new BPlusTree(rootDevice, childrenInfo.children[0].theLBA);
    assert(newTree);
    return true;
   }
   else
   {
    /* Create a new root with the children. */
    register class BlockCacheEntry*           newCacheEntry;
    register enum BlockCache::BlockCacheError cacheError;

    if (!BlockCache::getInstance().allocate(newCacheEntry, cacheError, transaction))
    {
     assert(0);
    }

    assert(cacheError == BlockCache::noError);
   
    register uint8_t*                    newNodeData   = newCacheEntry->getDataPointer();

    assert(newNodeData);

    memset(newNodeData, 0, sectorSize);

    register struct header* const        newHeader     = (struct header*) newNodeData;
    register struct firstLocation* const firstLocation = (struct firstLocation*) (newHeader + 1);
    register struct internalKey*         internalKey   = (struct internalKey*) (firstLocation + 1);

    register uint8_t                     newNbrOfKeys = 0;
    register uint16_t                    newSize = 0;

    if (childrenInfo.children[0].valid)
    {
     newSize = sizeof(struct internalLocation);

     toFileSystemEndian(&firstLocation->offset, sectorSize - newSize, isLittle); 

     register struct internalLocation*    internalLocation = (struct internalLocation*) (newNodeData + sectorSize - newSize);
     toFileSystemEndian(&internalLocation->theLBA, childrenInfo.children[0].theLBA.theLBA, isLittle);
     internalLocation--; 

     for(register unsigned int index = 1; index < 3; index++)
     {
      if (childrenInfo.children[index].valid)
      {
       internalKey->type = childrenInfo.children[index].key.type;
       toFileSystemEndian(&internalKey->major, childrenInfo.children[index].key.major, isLittle); 
       toFileSystemEndian(&internalKey->minor, childrenInfo.children[index].key.minor, isLittle);  

       newSize += sizeof(struct internalLocation);
       toFileSystemEndian(&internalKey->offset, sectorSize - newSize, isLittle); 

       toFileSystemEndian(&internalLocation->theLBA, childrenInfo.children[index].theLBA.theLBA, isLittle);
       internalLocation--;
       internalKey++;
       newNbrOfKeys++;
      }
     }
    }
    else
    {
     assert(!childrenInfo.children[1].valid);
     assert(!childrenInfo.children[2].valid);
    }

    newHeader->version = version | (isLittle ? 0 : bigEndian);
    newHeader->keys    = newNbrOfKeys;
    toFileSystemEndian(&newHeader->spaceUsedNFlags, newSize, isLittle);

    register enum FileSystem::FileSystemError fileSystemError;
    register LBA                              newLBA;

    if (!transaction->getFileSystem()->getAvailableLBA(newLBA, fileSystemError))
     assert(0); 

    assert(fileSystemError == FileSystem::noError);
     
    if (!newCacheEntry->setLBA(rootDevice, newLBA))
    {
     assert(0);
    }
    
    newCacheEntry->unlock(newNodeData, newCacheEntry, transaction);

    newTree = new BPlusTree(rootDevice, newLBA);

    assert(newTree);
    return true;
   }
  }
};

#endif
