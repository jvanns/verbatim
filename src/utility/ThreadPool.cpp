/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "ThreadPool.hpp"

using std::unique_ptr;
using boost::thread;

namespace verbatim {
namespace utility {

/*
 * Just execute a queue run
 */
void
Worker::operator()()
{
    pool.service.run();
}
 
// the constructor just launches some amount of workers
ThreadPool::ThreadPool(size_t threads) : running(false), work(service)
{
    workers.reserve(threads);

    for (size_t i = 0 ; i < threads ; ++i)
        workers.push_back(unique_ptr<thread>(new thread(Worker(*this))));

    running = true;
}
 
ThreadPool::~ThreadPool()
{
    stop();
}

void
ThreadPool::stop()
{
    if (!running)
        return;

    service.stop();

    for (size_t i = 0 ; i < workers.size() ; ++i)
        workers[i]->join();

    running = false;
}

} // utility
} // verbatim

