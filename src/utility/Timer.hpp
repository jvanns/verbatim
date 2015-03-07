/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_TIMER_HPP
#define VERBATIM_UTILITY_TIMER_HPP

// libc
#include <time.h>
#include <stdint.h>
#include <sys/time.h>

namespace verbatim {
namespace utility {

class Timer
{
    public:
        /* Member functions/methods */
        Timer();

        void start();
        void stop();
        size_t elapsed() const;
        static size_t precision(); // Returns the precision (eg nanoseconds)
    private:
        /* Member variables/attributes */
        bool running;
        timespec started, stopped;
};

} // utility
} // verbatim

#endif // VERBATIM_UTILITY_TIMER_HPP

