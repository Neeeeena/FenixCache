#ifndef  SUBTREEMETADATA_HPP
# define SUBTREEMETADATA_HPP

# include <stdint.h>
# include <assert.h>
# include <string.h>

# include <Serializable.hpp>

# include <Key.hpp>
# include <UUID.hpp>

# include <Globals.hpp>

class SubTreeMetaData : public Serializable
{
 private:
  struct MetaData
  {
   UUID     theUUID;
   uint64_t subTreeMajor;
   uint8_t  bits;
  };

 public:
  inline
  SubTreeMetaData()
  {
   subTreeMajor = 0;
   bits         = 0;
   theUUID      = { 0, 0};
  }

  inline
  SubTreeMetaData(register const UUID     theUUID,
                  register const uint64_t subTreeMajor,
                  register const uint8_t  bits)
  {
   this->theUUID      = theUUID;
   this->subTreeMajor = subTreeMajor;
   this->bits         = bits;
  }

  inline uint_fast16_t
  size(void) const
  {
   return sizeof(struct MetaData);
  }

  inline bool
  isSizeAcceptable(register const uint_fast16_t size) const
  {
   return size == this->size();
  }

  inline Key
  getKey(register const uint64_t index) const
  {
   const Key theKey = { .type  = fileSystemMetaDataType,
                        .major = fileSystemSubTreeKeyList,
                        .minor = index };

   return theKey;
  }

  inline bool
  toFileSystem(register uint8_t*   destination,
               register const bool isLittle) const
  {
   assert(destination);

   struct MetaData* const metaDataPtr = (struct MetaData*) destination;

   toFileSystemEndian(&metaDataPtr->theUUID.major, theUUID.major, isLittle);
   toFileSystemEndian(&metaDataPtr->theUUID.minor, theUUID.minor, isLittle);

   toFileSystemEndian(&metaDataPtr->subTreeMajor, subTreeMajor, isLittle);

   metaDataPtr->bits = bits;
   
   return true;
  }

  inline bool
  fromFileSystem(register const uint8_t*      source,
                 register const uint_fast16_t size,
                 register const bool          isLittle)
  {
   assert(source);

   const struct MetaData* const metaDataPtr = (struct MetaData*) source;

   theUUID.major = fromFileSystemEndian(&metaDataPtr->theUUID.major, isLittle);
   theUUID.minor = fromFileSystemEndian(&metaDataPtr->theUUID.minor, isLittle);

   subTreeMajor  = fromFileSystemEndian(&metaDataPtr->subTreeMajor, isLittle);

   bits = metaDataPtr->bits;

   return true;
  }

  inline UUID
  getUUID(void) const
  {
   return theUUID;
  }

  inline uint64_t
  getSubTreeMajor(void) const
  {
   return subTreeMajor;
  }

  inline unsigned int
  getMajorBitsUsed(void) const
  {
   return bits;
  }
 
 private:
  static const uint8_t      fileSystemMetaDataType   = 1;
  static const uint64_t     fileSystemSubTreeKeyList = 1;

  UUID     theUUID;
  uint64_t subTreeMajor;
  uint8_t  bits;
};

#endif
