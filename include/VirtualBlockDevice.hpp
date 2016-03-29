#ifndef VIRTUALBLOCKDEVICE_HPP
# define VIRTUALBLOCKDEVICE_HPP

# include <stdint.h>

# include <Globals.hpp>
# include <LBA.hpp>

class VirtualBlockDevice
{
 public:
  enum VirtualBlockDeviceError
  {
   noError = 0,
   deviceError
  };

  virtual bool
  readSector(register enum VirtualBlockDeviceError& error,
             register class BlockCacheEntry* const  cacheEntry,
             register const struct LBA              theLBA) = 0;

  virtual bool
  writeSector(register enum VirtualBlockDeviceError& error,
              register class BlockCacheEntry* const  cacheEntry,
              register const struct LBA              theLBA) = 0;
   
  virtual bool
  getSizeInSectors(register struct LBA& size) const = 0;
};

#endif
