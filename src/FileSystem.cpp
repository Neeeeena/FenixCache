#include <FileSystem.hpp>
#include <TransactionManager.hpp>
#include <VirtualBlockDeviceBroker.hpp>
#include <BPlusTree.hpp>

#include <SubTreeMetaData.hpp>
#include <SubTreeCount.hpp>

FileSystem::FileSystem(register const UUID theFSUUID,
                       register const UUID theVirtualBlockDeviceUUID)
{
 myUUID      = theFSUUID;
 blockDevice = 0;
 currentTree = 0;

 register enum VirtualBlockDeviceBroker::VirtualBlockDeviceBrokerError
 brokerError;
   
 if (!VirtualBlockDeviceBroker::getInstance().getVirtualBlockDevice(blockDevice,
                                                                    brokerError,
                                                                    theVirtualBlockDeviceUUID))
 {
  /*! \todo add error handling. */
  assert(0);
 }

 assert(blockDevice);

 if (!blockDevice->getSizeInSectors(maxLBA))
  assert(0);
 
 /* try transaction building the file system until it commits or permanently fails. */

 register class Transaction*
 transaction;

 register enum TransactionManager::TransactionManagerError
 transactionManagerError;

 if(!TransactionManager::getInstance().startTransaction(transaction, transactionManagerError, this))
 {
  /*! \todo add error handling. */
  assert(0);
 }

 assert(transactionManagerError == TransactionManager::noError);
 
 do
 {
  currentTree = new BPlusTree(blockDevice, this, transaction);

  assert(currentTree);
    
  /* Try to end transaction. */
  if (TransactionManager::getInstance().endTransaction(transactionManagerError, transaction))
   break;
 } while (transaction);

 assert(transactionManagerError == TransactionManager::noError);

 /* We need to do this is two steps as the first transaction is not
    set up correctly and cannot be used to add keys to the tree. */
 
 if(!TransactionManager::getInstance().startTransaction(transaction, transactionManagerError, this))
 {
  /*! \todo add error handling. */
  assert(0);
 }

 assert(transactionManagerError == TransactionManager::noError);
 
 do
 {
  register enum Transaction::TransactionError
  transactionError;
  
  /* Add empty subtree along with its meta data. */

  /* SubTree metadata. */
  register SubTreeMetaData metaData({.major = 0x8000000000000000,
                                     .minor = 0 }, 0, 4);

  if(!transaction->insert(transactionError, metaData, metaData.getKey(0)))
   assert(0);

  register SubTreeCount count(1);

  if(!transaction->insert(transactionError, count, count.getKey()))
   assert(0);
  
  /* Try to end transaction. */
  if (TransactionManager::getInstance().endTransaction(transactionManagerError, transaction))
   break;
 } while (transaction);

 assert(transactionManagerError == TransactionManager::noError); 
}

bool
FileSystem::updateTree(register const class BPlusTree* const newTree)
{
 if (currentTree)
  delete currentTree;

 currentTree = newTree;

 return true;
}
