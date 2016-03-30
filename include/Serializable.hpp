#ifndef  SERIALIZABLE_HPP
# define SERIALIZABLE_HPP

# include <stdint.h>

class Serializable
{
 public:
  virtual uint_fast16_t
  size(void) const = 0;

  virtual bool
  isSizeAcceptable(register const uint_fast16_t size) const
  {
   return size <= this->size();
  }

  virtual bool
  toFileSystem(register uint8_t*   destination,
               register const bool isLittle) const = 0;

  virtual bool
  fromFileSystem(register const uint8_t*      source,
                 register const uint_fast16_t size,
                 register const bool          isLittle) = 0;  
};

#endif
