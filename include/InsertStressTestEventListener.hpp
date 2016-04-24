/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef INSERTSTRESSTESTEVENTLISTENER_HPP
# define INSERTSTRESSTESTEVENTLISTENER_HPP

# include <assert.h>
# include <stdint.h>

# include <EventListener.hpp>

# include <EventListenerManager.hpp>
# include <UUID.hpp>
# include <FileSystemManager.hpp>
# include <TransactionManager.hpp>
# include <FileSystem.hpp>
# include <SubTreeTransaction.hpp>
# include <SubTreeBlobKey.hpp>

class InsertStressTestEventListener : public EventListener
{
 public:
  inline
  InsertStressTestEventListener()
  {
   alreadyRun = false;

   register enum EventListenerManager::EventListenerManagerError
   error;

   if (!EventListenerManager::getInstance().registerListener(error, this, __func__))
   {
    assert(0);
   }
   
   assert(error == EventListenerManager::noError);
  }

  inline virtual bool
  handleEvent(register unsigned int&            receiver,
              register class Event*&            outgoingEvent,
              register const unsigned int       sender,
              register const class Event* const incomingEvent)
  {
   
   /* Consume event when done with it. */
   delete incomingEvent;

   if (alreadyRun)
    return false;

   alreadyRun = true;

   register struct UUID fsUUID = {1, 0};
   register enum FileSystemManager::FileSystemManagerError
   fileSystemManagerError;

   class FileSystem* fileSystem = 0;
  
   /* Lookup the precreated file system. */ 
   if(!FileSystemManager::getInstance().getFileSystem(fileSystem, fileSystemManagerError, fsUUID))
   {
    assert(0);
   }

   assert(fileSystem);
   assert(fileSystemManagerError == FileSystemManager::noError); 

   register class SubTreeTransaction*
   transaction;

   register enum TransactionManager::TransactionManagerError
   transactionManagerError;

   register struct UUID subTreeUUID = {0x8000000000000000, 0};
 
   if(!TransactionManager::getInstance().startSubTreeTransaction(transaction, transactionManagerError, fileSystem, subTreeUUID))
   {
    /*! \todo add error handling. */
    assert(0);
   }

   assert(transactionManagerError == TransactionManager::noError);

   /* File operations can be done on sub tree. */

   register enum SubTreeTransaction::SubTreeTransactionError transactionError;
   struct SubTreeBlobKey subKey;
 
   if (!transaction->allocateBlob(subKey, transactionError, SubTreeBlobKey::data))
   {
    assert(0);
   }

   assert(transactionError == SubTreeTransaction::noError);

   for(unsigned int index = 0; index < 1000; index++)
   {
    const char test = 128 - index;

    /* Perform a test write on the SubTree. */
    if(!transaction->insertData(transactionError, (uint8_t*) &test, sizeof(test), subKey, index))
     assert(0);

    assert(transactionError == SubTreeTransaction::noError);

    /* Try to read it back! */
    char          readData = 0;
    uint_fast16_t readSize = sizeof(readData);

    if(!transaction->lookupData((uint8_t*)&readData, readSize, transactionError, subKey, index))
     assert(0);

    assert(transactionError == SubTreeTransaction::noError);
    assert(readSize == sizeof(readData));
    assert(readData == test);
   }

   /* End transaction. */
   if(!TransactionManager::getInstance().endSubTreeTransaction(transactionManagerError, transaction))
   {
    /*! \todo add error handling. */
    assert(0);
   }

   assert(transactionManagerError == TransactionManager::noError);

   register enum EventListenerManager::EventListenerManagerError
   error;

   if (!EventListenerManager::getInstance().deRegisterListener(error, this))
   {
    assert(0);
   }
   
   assert(error == EventListenerManager::noError);
   
   return false;
  }

 private:
  bool alreadyRun;
};

#endif
