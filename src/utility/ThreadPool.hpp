/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_THREADPOOL_HPP
#define VERBATIM_UTILITY_THREADPOOL_HPP

// boost
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

// stl
#include <vector>
#include <memory>

namespace verbatim {
namespace utility {
 
class ThreadPool;
 
class Worker {
    public:
        /* Member functions */
        Worker(ThreadPool &s) : pool(s) {}
        void operator()();
    private:
        /* Member attributes */
        ThreadPool &pool; 
};
 
class ThreadPool {
    public:
        /* Member functions */
        ThreadPool(size_t);
        ~ThreadPool();

        void stop(); // Will block
        template<typename Work> void submit(const Work &w);
    private:
        /* Member attributes */
        bool running;
        boost::asio::io_service service;
        boost::asio::io_service::work work;
        std::vector<std::unique_ptr<boost::thread> > workers;

        /* Friend class declarations */
        friend class Worker;
};

/*
 * Post work to the queue
 */
template<typename Work>
void ThreadPool::submit(const Work &w)
{
    service.post(w);
}

} // utility
} // verbatim

#endif // VERBATIM_UTILITY_THREADPOOL_HPP
