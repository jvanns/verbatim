/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Traverse.hpp"

// libc
#include <ftw.h>
#include <assert.h>

using std::endl;
using std::string;
using std::ostream;

namespace verbatim {

Traverse*
Traverse::traverser = NULL;

class Traverse::Dispatcher {
    public:
        inline void operator() (const Traverse::Path &p)
        {
            assert(Traverse::traverser != NULL);
            Traverse::traverser->delegate.dispatch(p);
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
        dispatch(verbatim::Traverse::Path(path, sb));
    }

    return 0; // Returning non-zero will terminate the nftw() traversal
}

} // C

namespace verbatim {

/* Methods/Member functions */
Traverse::Path::Path(const char *s, const struct stat *t) :
    name(new string(s)),
    info(new struct stat(*t))
{
}

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
    delegate.connect(callback, &Callback::operator());
}

void
Traverse::scan(const string &path)
{
    utility::Timer t;

    t.start();
    nftw(path.c_str(), &C::nftw_callback, 256, 0);
    t.stop();

    dispatch_scan_time = t.elapsed();
}

void
Traverse::print_metrics(ostream &stream) const
{
    stream <<
        "verbatim[Traverse]: Dispatch time = " <<
        dispatch_scan_time.seconds <<
        "s " <<
        dispatch_scan_time.nanoseconds <<
        "ns\n";
}

} // verbatim

