/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_TIMER_HPP
#define VERBATIM_UTILITY_TIMER_HPP

// libc
#include <stdint.h>
#include <stddef.h>

namespace verbatim {
namespace utility {

class Timer
{
    public:
        /* Type definitions */
        struct Duration
        {
            size_t seconds, nanoseconds;
            Duration() : seconds(0), nanoseconds(0) {}
        };

        /* Member functions/methods */
        Timer();
        void start();
        void stop();
        Duration elapsed() const;
    private:
        /* Type definitions */
        typedef Duration Timestamp;

        /* Member functions/methods */
        static Timestamp now();
        static Duration diff(const Timestamp &from, const Timestamp &to);

        /* Member variables/attributes */
        bool running;
        Timestamp started, stopped;
};

} // utility
} // verbatim

#endif // VERBATIM_UTILITY_TIMER_HPP

