/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "ThreadPool.hpp"

// libc
#include <assert.h>

using std::thread;
using std::unique_ptr;

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
ThreadPool::ThreadPool(size_t threads) :
    running(false),
    work(new boost::asio::io_service::work(service))
{
    workers.reserve(threads);
    start(threads);
}
 
ThreadPool::~ThreadPool()
{
    stop();
}

void
ThreadPool::restart()
{
    if (running)
        return;

    assert(!workers.empty());
    assert(!work);

    service.reset();
    work = new boost::asio::io_service::work(service);

    for (size_t i = 0 ; i < workers.size() ; ++i)
        workers[i] = unique_ptr<thread>(new thread(Worker(*this)));

    running = true;
}

void
ThreadPool::stop()
{
    if (!running)
        return;

    service.stop();

    for (size_t i = 0 ; i < workers.size() ; ++i)
        workers[i]->join();

    assert(work);
    delete work;
    work = NULL;

    running = false;
}

void
ThreadPool::wait()
{
    if (!running)
        return;

    /*
     * Wait until io_service.run() has no more work to do - the
     * threads will then return of their own accord but in any order.
     */
    assert(work);
    delete work;
    work = NULL;

    for (size_t i = 0 ; i < workers.size() ; ++i)
        workers[i]->join();

    service.stop();

    running = false;
}

void
ThreadPool::start(size_t threads)
{
    if (running)
        return;

    assert(workers.empty());

    for (size_t i = 0 ; i < threads ; ++i)
        workers.push_back(unique_ptr<thread>(new thread(Worker(*this))));

    running = threads > 0;
}

} // utility
} // verbatim

