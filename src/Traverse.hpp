/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_TRAVERSE_HPP
#define VERBATIM_TRAVERSE_HPP

// verbatim
#include "utility/Timer.hpp"

// Signals
#include "signals/Signal.h"

// libstdc++
#include <memory> // For shared_ptr<>
#include <string>
#include <iostream>

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
            const std::shared_ptr<const std::string> name;
            const std::shared_ptr<const struct stat> info;
            Path(const char *s, const struct stat *t);
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
        void print_metrics(std::ostream &stream) const;
    private:
        /* Attributes/member variables */
        static Traverse *traverser; // Required for libc func ptr callback :(
        Gallant::Signal1<const Path&> observers;
        utility::Timer::Duration dispatch_scan_time;

        /* Friend class declarations */
        friend class Dispatcher;
};

} // verbatim

#endif

