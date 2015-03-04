/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_STR2INT_HPP
#define VERBATIM_UTILITY_STR2INT_HPP

// verbatim
#include "Exception.hpp"

// libstdc++
#include <sstream>

// libc
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

namespace verbatim {
namespace utility {

template<typename Integer>
Integer
str2int(const char *s, const Integer *min = NULL, const Integer *max = NULL)
throw (ConversionError)
{
    char *end;
    long long result = 0;

    assert(s);

    errno = 0; // Be sure to reset errno here...
    result = strtoll(s, &end, 10); // strtol() very much depends on it

    if (result == LLONG_MIN && errno == ERANGE)
        throw ConversionError("str2int",
                              errno,
                              "Underflow detected attempting to convert %s.",
                              s);

    if (result == LLONG_MAX && errno == ERANGE)
        throw ConversionError("str2int",
                              errno,
                              "Overflow detected attempting to convert %s.",
                              s);

    if (end == s || (end && *end != '\0'))
        throw ConversionError("str2int",
                              errno,
                              "Invalid characters detected in %s.",
                              s);

    if (max && static_cast<Integer>(result) > *max) {
        std::ostringstream ss;
        ss << s << " exceeds maximum of " << *max;
        throw ConversionError("str2int", 0, ss.str().c_str());
    }

    if (min && static_cast<Integer>(result) < *min) {
        std::ostringstream ss;
        ss << s << " exceeds minimum of " << *min;
        throw ConversionError("str2int", 0, ss.str().c_str());
    }

    return static_cast<Integer>(result);
}

template<typename Integer>
std::string
int2str(const Integer &i)
{
    std::ostringstream ss;
    ss << i;
    return ss.str();
}

} // utility
} // verbatim

#endif
