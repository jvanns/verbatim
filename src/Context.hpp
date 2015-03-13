/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_CONTEXT_HPP
#define VERBATIM_CONTEXT_HPP

// verbatim
#include "Traverse.hpp"
#include "Database.hpp"
#include "utility/ThreadPool.hpp"

// libstdc++
#include <iostream>

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

        void run();
        inline Traverse& traverser() { return *tv; }
        inline Database& database()  { return *db; }
        void print_metrics(std::ostream &stream) const;
    private:
        /* Attributes/member variables */
        utility::ThreadPool *threads;
        Traverse *tv;
        Database *db;
};

} // verbatim

#endif
