#ifndef BLOCKDEVICE_HPP
# define BLOCKDEVICE_HPP

# include <stdio.h>
# include <assert.h>

# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <fcntl.h>

# include <Globals.hpp>
# include <LBA.hpp>

# include <VirtualBlockDevice.hpp>
# include <BlockCacheEntry.hpp>

class BlockDevice : public VirtualBlockDevice
{
 friend class OSInterface;
  
 public:
  inline bool
  readSector(register enum VirtualBlockDeviceError& error,
             register class BlockCacheEntry* const  cacheEntry,
             register const struct LBA              theLBA)
  {
   assert(devicefd != -1);
   assert(theLBA.theLBA < sectors);

   register const off_t off = lseek(devicefd, sectorSize * theLBA.theLBA, SEEK_SET);

   assert(off == sectorSize * theLBA.theLBA);
   assert(cacheEntry);
   
   register uint8_t* const data = cacheEntry->getDataPointer();
 
   assert(data);

   register const ssize_t readError = read(devicefd, data, sectorSize);

   assert(readError == sectorSize);

   error = noError;
   return true;
  }

  inline bool
  writeSector(register enum VirtualBlockDeviceError& error,
              register class BlockCacheEntry* const  cacheEntry,
              register const struct LBA              theLBA)
  {
   assert(devicefd != -1);
   assert(theLBA.theLBA < sectors);

   register const off_t off = lseek(devicefd, sectorSize * theLBA.theLBA, SEEK_SET);

   assert(off == sectorSize * theLBA.theLBA);
   assert(cacheEntry);
   
   register const uint8_t* const data = cacheEntry->getDataPointer();
 
   assert(data);

   register const ssize_t writeError = write(devicefd, data, sectorSize);

   assert(writeError == sectorSize);

   error = noError;
   return true;
  }

  inline bool
  getSizeInSectors(register struct LBA& size) const
  {
   size.theLBA = sectors;

   return sectors != 0;
  }
   
 private:
  inline
  BlockDevice(register const char* const devicePath)
  {
   sectors = 0;
   devicefd = open(devicePath, O_RDWR);

   assert(devicefd != -1);

   off_t off = lseek(devicefd, 0, SEEK_END);

   assert(off > 0);
   assert(off % sectorSize == 0);

   sectors = off / sectorSize;
  }

  int           devicefd;

  uint_fast64_t sectors;

};

#endif
