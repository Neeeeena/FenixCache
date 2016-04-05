/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef TRANSACTIONMANAGER_HPP
# define TRANSACTIONMANAGER_HPP

# include <assert.h>

# include <UUID.hpp>

# include <Transaction.hpp>
# include <SubTreeTransaction.hpp>

class TransactionManager
{
 public:
  enum TransactionManagerError
  {
   noError = 0,
   outOfTransactions,
   conflict, 
  };

  static inline TransactionManager& 
  getInstance()
  {
   return instance;
  }

  inline bool
  startSubTreeTransaction(register SubTreeTransaction* &         returnedTransaction,
                          register enum TransactionManagerError& error,
                          register class FileSystem* const       fileSystem,
                          register const struct UUID             subTreeUUID)
  {
   /*! \todo make this more efficient. */
   for(register unsigned int i = 0; i < maxSubTreeTransactions; i++)
   {
    if (subTreeTransactions[i].available)
    {
     subTreeTransactions[i].available = false;

     if (!subTreeTransactions[i].transaction.reinit(fileSystem, subTreeUUID))
      assert(0);

     returnedTransaction = &subTreeTransactions[i].transaction;
     
     error = noError;
 
     SubTreeObserverManager::getInstance().notifyStartTransaction(returnedTransaction, subTreeUUID);
     
     return true;	
    }
   }

   error = outOfTransactions;
   return false;
  }

  inline bool
  abortTransaction(register enum TransactionManagerError& error,
                   register SubTreeTransaction*&          transactionPtr)
  {
   assert(0);
   
   return false;
  }
  
  inline bool
  endSubTreeTransaction(register enum TransactionManagerError& error,
                        register SubTreeTransaction*&          transactionPtr)
  {
   /* check for conflict first. That way we can just return immediately. */
   if (transactionPtr->isConflicting())
   {
    assert(0);

    /*! \todo rewrite conflict code. */
    
    error = conflict;

    return false;
   }

   SubTreeObserverManager::getInstance().notifyEndTransaction(transactionPtr);
   if (!transactionPtr->end())
    assert(0);
   
   register const int index = ((uintptr_t) transactionPtr -
                               (uintptr_t) subTreeTransactions) /
                              sizeof(subTreeTransactions[0]);

   assert(index >= 0 && index < maxSubTreeTransactions);
   assert(&subTreeTransactions[index].transaction == transactionPtr);
   assert(!subTreeTransactions[index].available);

   error                         = noError;
   transactionPtr                = 0;
   subTreeTransactions[index].available = true;
      
   return true;  
  }
  
  inline bool
  startTransaction(register Transaction* &                returnedTransaction,
                   register enum TransactionManagerError& error,
		   register class FileSystem* const       fileSystem)
  {
   /*! \todo make this more efficient. */
   for(register unsigned int i = 0; i < maxTransactions; i++)
   {
    if (transactions[i].available)
    {
     transactions[i].available = false;

     if (!transactions[i].transaction.reinit(fileSystem))
      assert(0);

     returnedTransaction = &transactions[i].transaction;
     
     error = noError;
     return true;	
    }
   }

   error = outOfTransactions;
   return false;
  }

  inline bool
  abortTransaction(register enum TransactionManagerError& error,
                   register Transaction*&                 transactionPtr)
  {
   assert(0);
   return false;
  }
  
  inline bool
  endTransaction(register enum TransactionManagerError& error,
                 register Transaction*&                 transactionPtr)
  {
   /* check for conflict first. That way we can just return immediately. */
   if (transactionPtr->isConflicting())
   {
    if(!transactionPtr->abort())
     assert(0); 
    /*! \todo rewrite this so that fields are not inspected. */
    transactionPtr->reinit(transactionPtr->fileSystem);
    error = conflict;

    return false;
   }

   if (!transactionPtr->end())
    assert(0);
   
   register const int index = ((uintptr_t) transactionPtr -
                               (uintptr_t) transactions) /
                              sizeof(transactions[0]);

   assert(index >= 0 && index < maxTransactions);
   assert(&transactions[index].transaction == transactionPtr);
   assert(!transactions[index].available);

   error                         = noError;
   transactionPtr                = 0;
   transactions[index].available = true;
      
   return true;  
  }

 private:
  static const unsigned int
  maxTransactions = 16;

  static const unsigned int
  maxSubTreeTransactions = 16;
  
  static TransactionManager
  instance;

  struct
  {
   Transaction transaction;
   bool        available; 
  } transactions[maxTransactions];

  struct
  {
   SubTreeTransaction transaction;
   bool               available; 
  } subTreeTransactions[maxSubTreeTransactions];
  
  inline
  TransactionManager()
  {
   for(register unsigned int i = 0; i < maxTransactions; i++)
    transactions[i].available = true;   

   for(register unsigned int i = 0; i < maxSubTreeTransactions; i++)
    subTreeTransactions[i].available = true;   
  }
};

#endif
