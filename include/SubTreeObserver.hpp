/* Copyright (c) 1997-2016, FenixOS Developers
   All Rights Reserved.

   This file is subject to the terms and conditions defined in
   file 'LICENSE', which is part of this source code package.
 */

#ifndef SUBTREEOBSERVER_HPP
# define SUBTREEOBSERVER_HPP

# include <assert.h>
# include <stdint.h>

class SubTreeObserver
{
 public:
  virtual inline void
  notifyMountFileSystem(register class FileSystem* const  fileSystem)
  {
  }

  virtual inline void
  notifyUnmountFileSystem(register class FileSystem* const  fileSystem)
  {
  }

  virtual inline void
  notifyStartTransaction(register class Transaction* const transaction,
                         register const struct UUID        subTreeUUID)
  {
  }

  virtual inline void
  notifyAbortTransaction(register class Transaction* const transaction)
  {
  }

  virtual void
  notifyEndTransaction(register class Transaction* const transaction) = 0;

  virtual inline void
  notifyLookup(register class Transaction* const transaction,
               register const uint_fast16_t      size,
               register const uint64_t           subTreeId,
               register const uint64_t           major,
               register const uint64_t           minor)
  {
  }

  virtual inline void
  notifyInsert(register class Transaction* const transaction,
               register const uint_fast16_t      size,
               register const uint64_t           subTreeId,
               register const uint64_t           major,
               register const uint64_t           minor)
  {
  }

  virtual inline void
  notifyRemove(register class Transaction* const transaction,
               register const uint64_t           subTreeId,
               register const uint64_t           major,
               register const uint64_t           minor)
  {
  }

 private:
};

#endif
