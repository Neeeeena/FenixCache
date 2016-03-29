#ifndef SUBTREETRANSACTION_HPP
# define SUBTREETRANSACTION_HPP

# include <assert.h>
# include <stdint.h>

# include <SubTreeBlobKey.hpp>
# include <Transaction.hpp>
# include <Key.hpp>

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

   if (!Transaction::lookup(destination, size, transactionError, treeKey))
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

   if (!Transaction::insert(transactionError, source, size, treeKey))
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
  bitsUsed;

  uint64_t
  subTreeMajor;

  /*! \todo remove this when proper blob allocation routine has been added. */
  bool
  allocatedBlob;
  
  inline
  SubTreeTransaction()
  {
   bitsUsed      = 0;
   subTreeMajor  = 0;
   allocatedBlob = false;
  }

  inline bool
  reinit(register class FileSystem* const fileSystem,
	 register const struct UUID       subTreeUUID)
  {
   if (!Transaction::reinit(fileSystem))
    assert(0);

   /* Search through the meta data searching for the sub tree meta data. */

   uint64_t               subTreeCount = 0;
   register uint_fast16_t size = sizeof(subTreeCount);

   register enum TransactionError transactionError;
   register struct Key key = {.type  = FileSystem::fileSystemMetaDataType,
                              .major = FileSystem::fileSystemMetaDataMajor,
                              .minor = FileSystem::subTreeCount};             

   /*! \todo add template methods for accessing meta data. */
   if (!Transaction::lookup((uint8_t*)&subTreeCount, size, transactionError, key))
    assert(0);

   assert(size == sizeof(subTreeCount));
   assert(transactionError == Transaction::noError);

   for(register uint64_t index = 0; index < subTreeCount; index++)
   {
    struct SubTreeMetaData
    {
     UUID     theUUID;
     uint64_t subTreeMajor;
     uint8_t  bits;
    } metaData;

    key.type  = FileSystem::fileSystemMetaDataType;
    key.major = FileSystem::fileSystemSubTreeKeyList;
    key.minor = index;

    size = sizeof(metaData);
    
    /*! \todo add template methods for accessing meta data. */
    if (Transaction::lookup((uint8_t*)&metaData, size, transactionError, key))
    {
     assert(transactionError == Transaction::noError);
     assert(size == sizeof(metaData));

     if ((metaData.theUUID.major == subTreeUUID.major) &&
	 (metaData.theUUID.minor == subTreeUUID.minor))
     {
      this->subTreeMajor = metaData.subTreeMajor;
      this->bitsUsed     = metaData.bits;

      assert(this->bitsUsed);
      return true;
     }
    }   
   }
      
   return false;
  }
};

#endif
