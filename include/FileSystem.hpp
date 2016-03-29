#ifndef FILESYSTEM_HPP
# define FILESYSTEM_HPP

# include <assert.h>

# include <UUID.hpp>
# include <LBA.hpp>

class FileSystem
{
 friend class FileSystemManager;

 friend class SubTreeTransaction;
 public:
  enum FileSystemError
  {
   noError = 0,
   outOfSpace 
  };

  inline bool
  getCurrentTree(register const class BPlusTree* & returnedTree,
                 register enum FileSystemError&    error) const
  {
   returnedTree = currentTree;
   error        = noError;

   return true;
  }

  bool
  updateTree(register const class BPlusTree* const newTree);

  inline UUID
  getUUID(void) const
  {
   return myUUID;
  }

  inline bool
  getAvailableLBA(register struct LBA&           returnedLBA,
		  register enum FileSystemError& error)
  {
   if (theLBAClock.theLBA < maxLBA.theLBA)
   {   
    returnedLBA = theLBAClock;
    theLBAClock.theLBA++;
    error = noError;

    return true;
   }

   error = outOfSpace;
   return false;
  }

 private:
  static const uint8_t      rawDataType              = 0;
  static const uint8_t      fileSystemMetaDataType   = 1;

  static const uint64_t     fileSystemMetaDataMajor  = 0;
  static const uint64_t     fileSystemSubTreeKeyList = 1;

  static const uint64_t     subTreeCount             = 0;
  
  struct UUID               myUUID;
  class VirtualBlockDevice* blockDevice;
  const class BPlusTree*    currentTree;

  /*! \todo replace this with a proper garbage collector. */
  struct LBA                theLBAClock;
  struct LBA                maxLBA;
  
  /*! Create a new file system with UUID theFSUUID on device with UUID
      theVirtualBlockDeviceUUID. */
  FileSystem(register const UUID theFSUUID,
	     register const UUID theVirtualBlockDeviceUUID);

};

#endif
