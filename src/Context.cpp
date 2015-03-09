/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Context.hpp"

namespace verbatim {

Context::Context(size_t concurrency) : threads(NULL), tv(NULL), db(NULL)
{
    tv = new Traverse();
    threads = new utility::ThreadPool(concurrency);
    db = new Database(*tv, *threads);
}

Context::~Context()
{
    /*
     * Specific order. Do not change.
     */
    if (threads)
        threads->wait();

    delete threads;
    delete db;
    delete tv;
}

} // verbatim
