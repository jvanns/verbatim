/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_TRAVERSE_HPP
#define VERBATIM_TRAVERSE_HPP

// Signals
#include "signals/Signal.h"

// libstdc++
#include <string>

// libc
#include <sys/stat.h>

namespace verbatim {

class Traverse
{
    public:
        /*
         * Type definitions
         */
        struct Path {
            const char *name;
            const struct stat *info;
            Path(const char *s, const struct stat *t) : name(s), info(t) {}
        };

        struct Callback {
            virtual ~Callback() {}
            virtual void operator() (const Path&) = 0;
        };

        class Dispatcher; // Forward declaration only (for friend declaration)

        /* Methods/Member functions */
        Traverse();
        ~Traverse();

        void register_callback(Callback *callback);
        void scan(const std::string &path);
    private:
        /* Attributes/member variables */
        static Traverse *traverser; // Required for libc func ptr callback :(
        Gallant::Signal1<const Path&> observers;

        /* Friend class declarations */
        friend class Dispatcher;
};

} // verbatim

#endif

