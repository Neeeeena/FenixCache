#ifndef VIRTUALBLOCKDEVICEBROKER_HPP
# define VIRTUALBLOCKDEVICEBROKER_HPP

# include <assert.h>
# include <stdint.h>

# include <UUID.hpp>

# include <OSInterface.hpp>

class VirtualBlockDeviceBroker
{
 public:
  enum VirtualBlockDeviceBrokerError
  {
   noError = 0
  };

  static inline VirtualBlockDeviceBroker& 
  getInstance()
  {
   return instance;
  }

  inline bool
  getVirtualBlockDevice(register class VirtualBlockDevice*&          blockDevice,
			register enum VirtualBlockDeviceBrokerError& error,
			register const UUID                          theUUID) const
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
    
   register enum OSInterface::OSInterfaceError tmpError;
   
   if (OSInterface::getInstance().getVirtualBlockDevice(blockDevice, tmpError, theUUID))
   {
    error = noError;
    return true;
   }

   /*! \todo implement error handling. */
   assert(0);
  }

 private:
  inline
  VirtualBlockDeviceBroker()
  {
   for(register unsigned int i = 0; i < maxDevices; i++)
    devices[i].valid = false; 
  }

  static VirtualBlockDeviceBroker
  instance;

  static const unsigned int
  maxDevices = 16;

  struct
  {
   class VirtualBlockDevice* device;
   UUID                      uuid;
   bool                      valid;
  } devices[maxDevices];
};

#endif
