#ifndef  SUBTREECOUNT_HPP
# define SUBTREECOUNT_HPP

# include <stdint.h>
# include <assert.h>
# include <string.h>

# include <Serializable.hpp>

# include <Key.hpp>

# include <Globals.hpp>

class SubTreeCount : public Serializable
{
 public:
  inline
  SubTreeCount()
  {
   count = 0;
  }

  inline
  SubTreeCount(register const uint64_t count)
  {
   this->count = count;
  }

  inline uint_fast16_t
  size(void) const
  {
   return sizeof(uint64_t);
  }

  inline bool
  isSizeAcceptable(register const uint_fast16_t size) const
  {
   return size == this->size();
  }

  inline Key
  getKey(void) const
  {
   const Key theKey = { .type  = fileSystemMetaDataType,
                        .major = fileSystemMetaDataMajor,
                        .minor = subTreeCount };

   return theKey;
  }

  inline uint64_t
  getCount(void) const
  {
   return count;
  }

  inline bool
  toFileSystem(register uint8_t*   destination,
               register const bool isLittle) const
  {
   toFileSystemEndian((uint64_t*) destination, count, isLittle);
   
   return true;
  }

  inline bool
  fromFileSystem(register const uint8_t*      source,
                 register const uint_fast16_t size,
                 register const bool          isLittle)
  {
   count = fromFileSystemEndian((uint64_t*) source, isLittle);

   return true;
  }

 private:
  static const uint8_t      fileSystemMetaDataType   = 1;
  static const uint64_t     fileSystemMetaDataMajor  = 0;
  static const uint64_t     subTreeCount             = 0;

  uint64_t count;
};

#endif
