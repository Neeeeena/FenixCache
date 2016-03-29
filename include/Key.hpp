#ifndef KEY_HPP
# define KEY_HPP

# include <stdint.h>

struct Key
{
 uint_fast8_t  type;
 uint_fast64_t major;
 uint_fast64_t minor;
};

#endif
