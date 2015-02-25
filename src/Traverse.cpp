/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Traverse.hpp"

// libc
#include <ftw.h>
#include <assert.h>

using std::string;

namespace verbatim {

Traverse*
Traverse::traverser = NULL;

class Traverse::Dispatcher {
    public:
        inline void operator() (const Traverse::Path &p)
        {
            assert(Traverse::traverser != NULL);
            Traverse::traverser->observers(p);
        }
};

} // verbatim

namespace C {

/*
 * See man nftw for more info.
 */
extern "C"
int
nftw_callback(const char *path,
              const struct stat *sb,
              int flags,
              struct FTW *ftw)
{
    if (flags != FTW_NS) {
        static verbatim::Traverse::Dispatcher dispatch;
        const struct verbatim::Traverse::Path p(path, sb);

        dispatch(p);
    }

    return 0; // Returning non-zero will terminate the nftw() traversal
}

} // C

namespace verbatim {

/* Methods/Member functions */
Traverse::Traverse()
{
    Traverse::traverser = this; // Bind
}

Traverse::~Traverse()
{
    Traverse::traverser = NULL; // Unbind
}

void
Traverse::register_callback(Callback *callback)
{
    observers.Connect(callback, &Callback::operator());
}

void
Traverse::scan(const string &path)
{
    nftw(path.c_str(), &C::nftw_callback, 256, 0);
}

} // verbatim

