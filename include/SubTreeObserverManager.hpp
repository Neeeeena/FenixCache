/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef SUBTREEOBSERVERMANAGER_HPP
# define SUBTREEOBSERVERMANAGER_HPP

# include <assert.h>
# include <stdint.h>

# include <SubTreeObserver.hpp>

class SubTreeObserverManager
{
 public:
  static inline SubTreeObserverManager& 
  getInstance()
  {
   return instance;
  }

  inline bool
  registerObserver(register SubTreeObserver* const observer)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (!observers[i])
    {
     observers[i] = observer;
     return true;
    }
   }
   
   return false;
  }
  
  inline void
  notifyMountFileSystem(register class FileSystem* const  fileSystem)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyMountFileSystem(fileSystem);
   }
  }  

  inline void
  notifyUnmountFileSystem(register class FileSystem* const  fileSystem)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyUnmountFileSystem(fileSystem);
   }
  }  

  inline void
  notifyStartTransaction(register class Transaction* const transaction,
                         register const struct UUID        subTreeUUID)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyStartTransaction(transaction, subTreeUUID);
   }
  }

  inline void
  notifyAbortTransaction(register class Transaction* const transaction)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyAbortTransaction(transaction);
   }
  }

  inline void
  notifyEndTransaction(register class Transaction* const transaction)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyEndTransaction(transaction);
   }
  }


  inline void
  notifyLookup(register class Transaction* const transaction,
               register const uint_fast16_t      size,
               register const uint64_t           subTreeId,
               register const uint64_t           major,
               register const uint64_t           minor)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyLookup(transaction,
                                size,
                                subTreeId,
                                major,
                                minor);
   }
  }

  inline void
  notifyInsert(register class Transaction* const transaction,
               register const uint_fast16_t      size,
               register const uint64_t           subTreeId,
               register const uint64_t           major,
               register const uint64_t           minor)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyInsert(transaction,
                                size,
                                subTreeId,
                                major,
                                minor);
   }
  }

  inline void
  notifyRemove(register class Transaction* const transaction,
               register const uint64_t           subTreeId,
               register const uint64_t           major,
               register const uint64_t           minor)
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    if (observers[i])
     observers[i]->notifyRemove(transaction,
                                subTreeId,
                                major,
                                minor);
   }
  }

               
 private:
  static const unsigned int
  maxObservers = 16;

  static SubTreeObserverManager
  instance;

  SubTreeObserver*
  observers[maxObservers];

  inline
  SubTreeObserverManager()
  {
   for(register unsigned int i = 0; i < maxObservers; i++)
   {
    observers[i] = 0;
   }
  }
};

#endif
