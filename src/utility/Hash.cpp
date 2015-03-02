/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Hash.hpp"

// libc
#include <assert.h>

namespace {

/*
 * The following two hash algorithms are implementations of the common FNV-1
 * hash algorithm as described here;
 *
 * http://isthe.com/chongo/tech/comp/fnv/
 * http://en.wikipedia.org/wiki/Fowler_Noll_Vo_hash#Notes
 *
 */
size_t
fnv32(const char *s, size_t len)
{
    static const uint32_t offset = 2166136261U,
                          prime = 16777619U; 
    uint32_t hash = offset;

    assert(sizeof(size_t) == sizeof(uint32_t));
    assert(s);

    for (len += 1 ; len > 0 ; --len)
        hash = (hash * prime) ^ *s++;

    return hash;
}

size_t
fnv64(const char *s, size_t len)
{
    static const uint64_t offset = 14695981039346656037U,
                          prime = 1099511628211U; 
    uint64_t hash = offset;

    assert(sizeof(size_t) == sizeof(uint64_t));
    assert(s);

    for (len += 1 ; len > 0 ; --len)
        hash = (hash * prime) ^ *s++;

    return hash;
}

} // anonymous

namespace verbatim {
namespace utility {

const Hash::Hasher Hash::hashers[2] = {&fnv32, &fnv64};
const int Hash::index = sizeof(size_t) == 4 ? 0 : // 32-bit
                        sizeof(size_t) == 8 ? 1 : // 64-bit
                        -1;

size_t
Hash::operator() (const char *s, size_t len) const
{
    assert(Hash::index != -1);
    return ((*hashers[Hash::index])(s, len));
}

} // utility
} // verbatim
