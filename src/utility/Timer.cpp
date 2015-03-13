/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Timer.hpp"

// libc
#include <time.h>
#include <sys/time.h>

namespace verbatim {
namespace utility {

Timer::Timer() : running(false)
{
}

void
Timer::start()
{
    started = now();
    running = true;
}

void
Timer::stop()
{
    stopped = now();
    running = false;
}

Timer::Duration
Timer::elapsed() const
{
    if (!running)
        return diff(stopped, started);

    return diff(now(), started);
}

inline
Timer::Timestamp
Timer::now()
{
    timespec ts = {0};
    Timer::Timestamp t;

    clock_gettime(CLOCK_REALTIME, &ts);

    t.seconds = ts.tv_sec;
    t.nanoseconds = ts.tv_nsec;

    return t;
}

inline 
Timer::Duration
Timer::diff(const Timestamp &from, const Timestamp &to)
{
    Timer::Duration d;

    d.seconds = from.seconds - to.seconds;
    d.nanoseconds = from.nanoseconds - to.nanoseconds;

    return d;
}

} // utility
} // verbatim
