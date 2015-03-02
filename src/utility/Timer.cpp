/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Timer.hpp"

namespace {

inline timespec
now()
{
    timespec ts = {0};
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts;
}

inline size_t
diff(const timespec &from, const timespec &to)
{
    return ((from.tv_sec - to.tv_sec) * 1000000000UL)
          + (from.tv_nsec - to.tv_nsec);
}

} // anonymous

namespace verbatim {
namespace utility {

Timer::Timer() : running(false)
{
    started.tv_sec = 0;
    started.tv_nsec = 0;
    stopped.tv_sec = 0;
    stopped.tv_nsec = 0;
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

size_t
Timer::elapsed() const
{
    if (!running)
        return diff(stopped, started);

    return diff(now(), started);
}

} // utility
} // verbatim
