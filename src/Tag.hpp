/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_TAG_HPP
#define VERBATIM_TAG_HPP

// boost serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>

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

    template<typename Archive>
    void
    serialize(Archive &archive,
              unsigned int /* version */)
    {
        archive
            & modified
            & file
            & artist
            & album
            & title
            & genre;
    }
};

} // verbatim

#endif
