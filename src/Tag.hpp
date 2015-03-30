/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_TAG_HPP
#define VERBATIM_TAG_HPP

// boost serialization
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

// libstdc++
#include <string>
#include <vector>
#include <iostream>

// libc
#include <time.h> // For time_t

namespace verbatim {

struct Img
{
    /* Type definitions */
    typedef std::vector<char> ByteVector;

    size_t size;
    ByteVector data;
    std::string mimetype;

    /* Member functions/methods */
    Img() : size(0) {}

    template<typename Archive>
    void
    serialize(Archive &archive,
              unsigned int /* version */)
    {
        archive
            & size
            & data
            & mimetype;
    }
};

struct Tag
{
    /* Member variables/attributes */
    time_t modified;        // Modification time of file content (eg. tags)
    std::string artist,     // Performing artist
                album,      // EP/LP/Single/Album name
                title,      // Track title
                genre,      // Apparent genre
                filename;   // Source filename

    /* Member functions/methods */
    Tag() : modified(0) {}

    template<typename Archive>
    void
    serialize(Archive &archive,
              unsigned int /* version */)
    {
        archive
            & modified
            & artist
            & album
            & title
            & genre
            & filename;
    }
};

std::ostream& operator<< (std::ostream &s, const Img &i);
std::ostream& operator<< (std::ostream &s, const Tag &t);

} // verbatim

#endif
