#ifndef GLOBALS_HPP
# define GLOBALS_HPP

#include <assert.h>
#include <stdint.h>

/*! \todo global todo: sort structs after natural alignment */
/*! \todo global todo: check that class members are sorted after natural alignment */
/*! \todo global todo: add const on methods when at all relevant */
/*! \todo global todo: move class field members to the top */
/*! \todo global todo: scatter register and const */
/*! \todo global todo: endianness for meta data is currently only little endian. This must change. */
/*! \todo global todo: review the friends of each class removing friends if possible. */

static const unsigned int
sectorSize = 4096;

#if __GNUC__
#else
#error Must be compiled with GNU g++
#endif

static inline void
toFileSystemEndian(register uint16_t* const ptr,
		   register const uint16_t  value,
		   register const bool      isLittle)
{
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  while (!isLittle)
  {
   assert(0);
  }

  *ptr = value;
# else
# error Big endian not supported
# endif
}

static inline uint16_t
fromFileSystemEndian(register const uint16_t* const ptr,
 		     register const bool            isLittle)
{
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  while (!isLittle)
  {
   assert(0);
  }

  return *ptr;
# else
# error Big endian not supported
# endif
}

static inline void
toFileSystemEndian(register uint32_t* const ptr,
		   register const uint32_t  value,
		   register const bool      isLittle)
{
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  while (!isLittle)
  {
   assert(0);
  }

  *ptr = value;
# else
# error Big endian not supported
# endif
}

static inline uint32_t
fromFileSystemEndian(register const uint32_t* const ptr,
 		     register const bool            isLittle)
{
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  while (!isLittle)
  {
   assert(0);
  }

  return *ptr;
# else
# error Big endian not supported
# endif
}

static inline void
toFileSystemEndian(register uint64_t* const ptr,
		   register const uint64_t  value,
		   register const bool      isLittle)
{
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  while (!isLittle)
  {
   assert(0);
  }

  *ptr = value;
# else
# error Big endian not supported
# endif
}

static inline uint64_t
fromFileSystemEndian(register const uint64_t* const ptr,
 		     register const bool            isLittle)
{
# if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  while (!isLittle)
  {
   assert(0);
  }

  return *ptr;
# else
# error Big endian not supported
# endif
}


#endif
