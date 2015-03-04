/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_TAG_HPP
#define VERBATIM_TAG_HPP

// libstdc++
#include <string>

// libc
#include <time.h> // For time_t

namespace verbatim {

struct Tag
{
    time_t modified;    // Modification time of file content (eg. tags)
    std::string file,   // Filename of original source
                artist, // Performing artist
                album,  // EP/LP/Single/Album name
                title,  // Track title
                genre;  // Apparent genre

    Tag() : modified(0) {}
};

} // verbatim

#endif
