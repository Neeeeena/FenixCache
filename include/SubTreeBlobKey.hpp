#ifndef SUBTREEBLOBKEY_HPP
# define SUBTREEBLOBKEY_HPP

# include <stdint.h>

struct SubTreeBlobKey
{
 enum Type
 {
  data
 };
  
 uint_fast64_t major;
};

#endif
