#ifndef  SUBTREETRANSACTION_HPP
# define SUBTREETRANSACTION_HPP

# include <assert.h>
# include <stdint.h>

# include <SubTreeBlobKey.hpp>
# include <Transaction.hpp>
# include <Key.hpp>
# include <Serializable.hpp>

# include <RawData.hpp>
# include <SubTreeCount.hpp>
# include <SubTreeMetaData.hpp>

class SubTreeTransaction : protected Transaction
{
 friend class TransactionManager;
  
 public:
  enum SubTreeTransactionError
  {
   noError = 0
  };    
  
  inline bool
  allocateBlob(register struct SubTreeBlobKey&          returnedKey,
               register enum SubTreeTransactionError&   error,
               register const enum SubTreeBlobKey::Type subTreeType)
  {
   /*! \todo replace this dummy implementation. */
   assert(subTreeType == SubTreeBlobKey::data);
   assert(!allocatedBlob);

   returnedKey.major = 0;

   error = noError;
   return true;
  }

  inline bool
  lookupData(register uint8_t* const                destination,
             register uint_fast16_t&                size,
             register enum SubTreeTransactionError& error,
             register const struct SubTreeBlobKey   key,
             register const uint_fast64_t           minor)
  {
   /*! \todo update this dummy implementation */
   register struct Key treeKey;

   treeKey.type  = FileSystem::rawDataType;
   treeKey.major = key.major;
   treeKey.minor = minor;

   enum Transaction::TransactionError transactionError;

   register RawData rawData(destination, size);

   if (!Transaction::lookup(rawData, size, transactionError, treeKey))
    assert(0);
     
   assert(transactionError == Transaction::noError);

   error = noError;
   return true;
  }

  inline bool
  insertData(register enum SubTreeTransactionError& error,
             register const uint8_t* const          source,
             register const uint_fast16_t           size,
             register const struct SubTreeBlobKey   key,
             register const uint_fast64_t           minor)
  {
   /*! \todo update this dummy implementation */
   register struct Key treeKey;

   treeKey.type  = FileSystem::rawDataType;
   treeKey.major = key.major;
   treeKey.minor = minor;

   enum Transaction::TransactionError transactionError;

   register const RawData rawData((uint8_t*)source, size);

   if (!Transaction::insert(transactionError, rawData, treeKey))
    assert(0);
     
   assert(transactionError == Transaction::noError);

   error = noError;
   return true;
  }

  inline bool
  remove(register enum SubTreeTransactionError& error,
         register const struct SubTreeBlobKey   key,
         register const uint_fast64_t           minor)
  {
   assert(0);

   return false;
  }
  
 private:
  unsigned int
  majorBitsUsed;

  uint64_t
  subTreeMajor;

  /*! \todo remove this when proper blob allocation routine has been added. */
  bool
  allocatedBlob;
  
  inline
  SubTreeTransaction()
  {
   majorBitsUsed = 0;
   subTreeMajor  = 0;
   allocatedBlob = false;
  }

  inline bool
  reinit(register class FileSystem* const fileSystem,
         register const struct UUID       subTreeUUID)
  {
   if (!Transaction::reinit(fileSystem))
    assert(0);

   register SubTreeCount  count;
   register uint_fast16_t size;

   /* Search through the meta data searching for the sub tree meta data. */
   register enum TransactionError transactionError;

   if (!Transaction::lookup(count, size, transactionError, count.getKey()))
    assert(0);

   assert(transactionError == Transaction::noError);

   for(register uint64_t index = 0; index < count.getCount(); index++)
   {
    register SubTreeMetaData metaData;
    
    if (Transaction::lookup(metaData, size, transactionError, metaData.getKey(index)))
    {
     assert(transactionError == Transaction::noError);

     if ((metaData.getUUID().major == subTreeUUID.major) &&
         (metaData.getUUID().minor == subTreeUUID.minor))
     {
      this->subTreeMajor  = metaData.getSubTreeMajor();
      this->majorBitsUsed = metaData.getMajorBitsUsed();

      assert(this->majorBitsUsed);
      return true;
     }
    }   
   }
      
   return false;
  }
};

#endif
