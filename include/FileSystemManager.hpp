#ifndef FILESYSTEMMANAGER_HPP
# define FILESYSTEMMANAGER_HPP

# include <assert.h>

# include <FileSystem.hpp>
# include <UUID.hpp> 

class FileSystemManager
{
 public:
  enum FileSystemManagerError
  {
   noError = 0,
   fileSystemNotFound
  };

  static inline FileSystemManager& 
  getInstance(void)
  {
   return instance;
  }

  inline bool
  getFileSystem(register class FileSystem* &          fileSystem,
		register enum FileSystemManagerError& error,
		register const UUID                   theUUID)
  {
   for(register unsigned int i = 0; i < maxFileSystems; i++)
   {
    if (fileSystems[i].valid)
    {
     assert(fileSystems[i].fileSystem);

     const struct UUID fsUUID = fileSystems[i].fileSystem->getUUID();
     
     if ((fsUUID.major == theUUID.major) &&
	 (fsUUID.minor == theUUID.minor))
     {
      error = noError;
      fileSystem = fileSystems[i].fileSystem;
      
      return true;
     }
    }
   }   

   error = fileSystemNotFound;
   return false; 
  }

 private:
  static const unsigned int
  maxFileSystems = 16;
  
  static FileSystemManager
  instance;

  struct
  {
   bool        valid;
   FileSystem* fileSystem;
  } fileSystems[maxFileSystems];

  inline
  FileSystemManager()
  {
   for(register unsigned int i = 0; i < maxFileSystems; i++)
    fileSystems[i].valid = false;

   /*! \todo query devices and create file systems. 
       Right now the code just creates a new file system. */

   register const UUID tmpFSUUID                 = {1, 0};
   register const UUID tmpVirtualBlockDeviceUUID = {0, 0};

   fileSystems[0].fileSystem = new FileSystem(tmpFSUUID,
					      tmpVirtualBlockDeviceUUID);
 
   assert(fileSystems[0].fileSystem);

   fileSystems[0].valid = true;
  }
};

#endif
