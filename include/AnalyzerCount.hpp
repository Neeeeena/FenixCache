#ifndef  ANALYZERCOUNT_HPP
# define ANALYZERCOUNT_HPP

# include <stdint.h>
# include <assert.h>
# include <string.h>

# include <Serializable.hpp>

# include <Key.hpp>

# include <Globals.hpp>

class AnalyzerCount : public Serializable
{
 public:
  inline
  AnalyzerCount()
  {
   count = 0;
  }

  inline
  AnalyzerCount(register const uint64_t count)
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
   const Key theKey = { .type  = 255,
                        .major = 0,
                        .minor = 0 };

   return theKey;
  }

  inline uint64_t
  getCount(void) const
  {
   return count;
  }

  inline void
  increment(void)
  {
   count++;
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
  uint64_t count;
};

#endif
