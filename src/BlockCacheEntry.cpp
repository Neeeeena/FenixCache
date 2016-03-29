#include <BlockCacheEntry.hpp>

#include <BlockCache.hpp>

bool
BlockCacheEntry::setLBA(register VirtualBlockDevice* device,
                        register const struct LBA    lba)
{
 for(register unsigned int i = 0; i < maxLocations; i++)
 {
  if (!locations[i].valid)
  {
   locations[i].device        = device;
   locations[i].lba           = lba;
   locations[i].valid         = true;
   locations[i].transactional = allocated;

   BlockCache::getInstance().addToHashTable(this, i);    
   break;
  }

  assert(i < (maxLocations - 1));

  if (i >= (maxLocations - 1))
  {
   return false;
  }
 }
  
 return true;
}

