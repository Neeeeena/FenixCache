#include <assert.h>
#include <string.h>

#include <BPlusTree.hpp>
#include <Transaction.hpp>
#include <BlockCacheEntry.hpp>
#include <BlockCache.hpp>
#include <FileSystem.hpp>

BPlusTree::BPlusTree(register class VirtualBlockDevice* const device,
	             register class FileSystem* const         fileSystem,
	             register class Transaction* const        transaction)
{
 rootDevice     = device;

 register class BlockCacheEntry*           cacheEntry;
 register enum BlockCache::BlockCacheError cacheError;

 if (!BlockCache::getInstance().allocate(cacheEntry, cacheError, transaction))
 {
  assert(0);
 }
    
 assert(cacheError == BlockCache::noError);
   
 register uint8_t* dataPointer = cacheEntry->getDataPointer();

 assert(dataPointer);

 /* Create an empty root. */
 memset(dataPointer, 0, sectorSize);

 ((struct header*) dataPointer)->version = version;
 ((struct header*) dataPointer)->keys    = 0;
 toFileSystemEndian(&((struct header*) dataPointer)->spaceUsedNFlags, 0 | isLeaf, true);

 register enum FileSystem::FileSystemError fileSystemError;

 if (!fileSystem->getAvailableLBA(rootLBA, fileSystemError))
  assert(0); 
   
 if (!cacheEntry->setLBA(rootDevice, rootLBA))
 {
  assert(0);
 }
    
 cacheEntry->unlock(dataPointer, cacheEntry, transaction);
}
