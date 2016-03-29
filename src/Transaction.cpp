#include <Transaction.hpp>

#include <BPlusTree.hpp>
#include <BlockCacheEntry.hpp>

bool
Transaction::lookup(register uint8_t* const         destination,
                    register uint_fast16_t&         size,
                    register enum TransactionError& error,
                    register const Key              key)
{
 register enum BPlusTree::BPlusTreeError bPlusTreeError;

 assert(currentTree);

 if(!currentTree->lookup(destination, size, bPlusTreeError, key, this))
  assert(0);

 error = noError;
 return true; 
}


bool
Transaction::insert(register enum TransactionError&         error,
                    register const uint8_t* const           source,
                    register const uint_fast16_t            size,
                    register const Key                      key)
{
 register enum BPlusTree::BPlusTreeError bPlusTreeError;

 assert(currentTree);

 register const BPlusTree* oldTree = currentTree;

 if(!oldTree->insert(currentTree, bPlusTreeError, source, size, key, this))
  assert(0);

 if (oldTree != originalTree)
  delete oldTree;

 error = noError;
 return true;
}

bool
Transaction::end(void)
{
 assert(fileSystem);

 if (currentTree != originalTree)
  fileSystem->updateTree(currentTree); 

 fileSystem      = 0;
 originalTree    = 0;
 currentTree     = 0;

 /* Loop through the entries removing their transactional status. */
 for(;
     allocatedEntries;
     allocatedEntries = allocatedEntries->next)
 {
  register BlockCacheEntry* const tmp = allocatedEntries;

  for(register unsigned int location = 0;
      location < BlockCacheEntry::maxLocations;
      location++)
  {
   tmp->locations[location].transactional = false;
  } 
    
  tmp->allocated   = false;
  tmp->transaction = 0;
 }

 allocatedEntries = 0;
   
 return true;
}