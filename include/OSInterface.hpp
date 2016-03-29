#ifndef OSINTERFACE_HPP
# define OSINTERFACE_HPP

# include <assert.h>
# include <stdint.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <fcntl.h>

# include <Globals.hpp>
# include <UUID.hpp>

# include <BlockDevice.hpp>

class OSInterface
{
 public:
  enum OSInterfaceError
  {  
   noError = 0,
   deviceNotFound
  };

  static inline OSInterface& 
  getInstance()
  {
   return instance;
  }

  inline bool
  getVirtualBlockDevice(register class VirtualBlockDevice*& blockDevice,
			register enum OSInterfaceError&     error,
			register UUID                       theUUID)
  {
   /*! \todo Make this more efficient. */
   for(register unsigned int i = 0; i < maxDevices; i++)
   {
    if (devices[i].valid &&
	devices[i].uuid.major == theUUID.major &&
	devices[i].uuid.minor == theUUID.minor)
    {
     blockDevice = devices[i].device;
     error = noError;

     return true;
    }	
   }

   assert(0);

   error = deviceNotFound;
   return false;
  }

  /* Very crude for now. */
  inline bool
  getOSEvent(register const class VirtualBlockDevice*& blockDevice,
	     register uint32_t&                        tick,
	     register enum OSInterfaceError&           error,
	     register bool&                            newDevice)
  {
   tick = this->tick++;
   error = noError;

   while(deviceClock < maxDevices)
   {
    deviceClock++;

    if(devices[deviceClock - 1].valid)
    {
     blockDevice = devices[deviceClock - 1].device;
     newDevice = true;
    }
   }

   newDevice = false;
   return true;    
  }

 private:
  static OSInterface
  instance;

  static const unsigned int
  maxDevices = 16;

  struct
  {
   const char*         devicePath;
   VirtualBlockDevice* device;
   UUID                uuid;
   bool                valid;
  } devices[maxDevices];

  unsigned int
  deviceClock;

  uint32_t
  tick;

  inline void
  initDevice(register const unsigned int device,
             register const unsigned int sectors)
  {
   int fd = open(devices[device].devicePath, O_CREAT | O_TRUNC | O_RDWR, 0700);

   assert (fd != -1);

   off_t off = lseek(fd, sectors * sectorSize - 1, SEEK_SET);
 
   assert(off == sectors * sectorSize - 1);

   char tmp = 0;

   ssize_t size = write(fd, &tmp, 1);

   assert(size == 1);

   int error = close(fd);

   assert(error != -1);
  }

  inline
  OSInterface()
  {
   for(register unsigned int i = 0; i < maxDevices; i++)
   {
    devices[i].valid = false;
   }

   /* Create the devices we need for experiments. */

   devices[0].devicePath = "devices/dev0";
   devices[0].device = 0;
   devices[0].uuid.major = 0;
   devices[0].uuid.minor = 0;

   initDevice(0, 16 * 1024);

   devices[0].device = new BlockDevice(devices[0].devicePath);

   assert(devices[0].device);

   devices[0].valid = true;

   deviceClock = 0;
   tick = 0;
  }
};

#endif
