#ifndef TRANSACTION_HPP
# define TRANSACTION_HPP

# include <assert.h>

# include <UUID.hpp>
# include <FileSystem.hpp>
# include <Key.hpp>
# include <Serializable.hpp>

class Transaction
{
 friend class TransactionManager;
 friend class SubTreeTransaction;
  
 public:
  enum TransactionError
  {
   noError = 0,
   keyNotFound,
   dataTooBig,
   sizeIsNotAcceptable
  };    

  inline class BlockCacheEntry*
  addToTransaction(register class BlockCacheEntry* const entry)
  {
   class BlockCacheEntry* returnValue = allocatedEntries;

   allocatedEntries = entry;
   
   return returnValue;
  }

  virtual inline class FileSystem*
  getFileSystem(void) const
  {
   assert(fileSystem);

   return fileSystem;
  }

  bool
  lookup(register Serializable&          destination,
         register uint_fast16_t&         size,
         register enum TransactionError& error,
         register const Key              key);
  
  bool
  insert(register enum TransactionError& error,
         register const Serializable&    source,
         register const Key              key);

  bool
  remove(register enum TransactionError& error,
         register const Key              key);
  
 private:
  const class BPlusTree*
  originalTree;

  const class BPlusTree*
  currentTree;
  
  class FileSystem*
  fileSystem;

  class BlockCacheEntry*
  allocatedEntries;
   
  inline
  Transaction()
  {
   originalTree     = 0;
   currentTree      = 0;
   fileSystem       = 0;
   allocatedEntries = 0;
  }
  
  inline bool
  isConflicting(void) const
  {
   if (this->currentTree == originalTree)
    return false; 

   enum FileSystem::FileSystemError error;
   const class BPlusTree*           currentTree;                
   
   if(!fileSystem->getCurrentTree(currentTree, error))
   {
    assert(0);
   }
   
   if (currentTree == originalTree)
    return false;

   return true;
  }

  bool
  end(void);
  
  inline bool
  reinit(register class FileSystem* const fileSystem)
  {
   assert(fileSystem);
   assert(!this->fileSystem);
   assert(!originalTree);
   assert(!allocatedEntries);

   if (this->fileSystem)
    return false;

   this->fileSystem = fileSystem;
   originalTree     = 0;
   
   enum FileSystem::FileSystemError error;

   if(!fileSystem->getCurrentTree(originalTree, error))
   {
    assert(0);
    return false;
   }

   currentTree = originalTree;
   return true;
  }

  virtual inline bool
  abort(void)
  {
   assert(0);
   return false;
  }
};

#endif
