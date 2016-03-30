#ifndef  RAWDATA_HPP
# define RAWDATA_HPP

# include <stdint.h>
# include <assert.h>
# include <string.h>

# include <Serializable.hpp>

class RawData : public Serializable
{
 public:
  inline
  RawData()
  {
   this->data   = 0;
   this->mySize = 0;
  }

  inline
  RawData(register uint8_t* const       data,
          register const uint_fast16_t  size)
  {
   this->data   = data;
   this->mySize = size;
  }

  inline uint_fast16_t
  size(void) const
  {
   return mySize;
  }

  inline bool
  toFileSystem(register uint8_t*   destination,
               register const bool isLittle) const
  {
   assert(data);
   assert(destination);

   if(mySize)
    memcpy(destination, data, mySize);
   
   return true;
  }

  inline bool
  fromFileSystem(register const uint8_t*      source,
                 register const uint_fast16_t size,
                 register const bool          isLittle)
  {
   assert(data);
   assert(source);

   if (size)
    memcpy(data, source, size);

   return true;
  }

 private:
  uint8_t*      data;
  uint_fast16_t mySize; 
};

#endif
