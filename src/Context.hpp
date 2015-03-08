/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_CONTEXT_HPP
#define VERBATIM_CONTEXT_HPP

// verbatim
#include "Traverse.hpp"
#include "Database.hpp"
#include "utility/ThreadPool.hpp"

namespace verbatim {

/*
 * Main program context
 */
class Context
{
    public:
        /* Methods/Member functions */
        Context(size_t concurrency);
        ~Context();

        inline Traverse& traverser() { return *tv; }
        inline Database& database()  { return *db; }
    private:
        /* Attributes/member variables */
        utility::ThreadPool *threads;
        Traverse *tv;
        Database *db;
};

} // verbatim

#endif
