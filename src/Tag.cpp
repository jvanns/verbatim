/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Tag.hpp"

using std::ostream;

namespace verbatim {

ostream&
operator<< (ostream &s, const Img &i)
{
    s <<
        i.mimetype << '\t' <<
        i.size;

    return s;
}

ostream&
operator<< (ostream &s, const Tag &t)
{
    s << 
        t.filename << '\t' <<
        t.modified << '\t' <<
        t.genre << '\t' <<
        t.artist << '\t' <<
        t.album << '\t' <<
        t.title;

    return s;
}

} // verbatim
