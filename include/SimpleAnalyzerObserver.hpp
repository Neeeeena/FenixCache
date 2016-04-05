/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef SIMPLEANALYZEROBSERVER_HPP
# define SIMPLEANALYZEROBSERVER_HPP

# include <assert.h>
# include <stdint.h>

# include <SubTreeObserver.hpp>
# include <SubTreeObserverManager.hpp>

# include <AnalyzerCount.hpp>
# include <Transaction.hpp>

class SimpleAnalyzerObserver : public SubTreeObserver
{
 public:
  inline
  SimpleAnalyzerObserver()
  {
   SubTreeObserverManager::getInstance().registerObserver(this);
  }

  void
  notifyEndTransaction(register class Transaction* const transaction)
  {
   /* Use a custom Serializable to read and write data to the file system. */
   register AnalyzerCount
   counter;

   register enum Transaction::TransactionError
   transactionError;

   register uint_fast16_t
   size;

   if(!transaction->lookup(counter, size, transactionError, counter.getKey()))
    assert(transactionError == Transaction::keyNotFound);
   
   counter.increment();
   
   if(!transaction->insert(transactionError, counter, counter.getKey()))
    assert(0);
  }

 private:
};

#endif
