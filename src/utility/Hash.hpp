/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_HASH_HPP
#define VERBATIM_UTILITY_HASH_HPP

// libc
#include <stdint.h>
#include <stddef.h>

namespace verbatim {
namespace utility {

struct Hash
{
    size_t operator() (const char *s, size_t len) const;
    typedef size_t (*Hasher)(const char*, size_t);
    static const Hasher hashers[2];
    static const int index;
};

} // utility
} // verbatim

#endif // VERBATIM_UTILITY_HASH_HPP
