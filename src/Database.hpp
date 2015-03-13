/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_DATABASE_HPP
#define VERBATIM_DATABASE_HPP

// verbatim
#include "Traverse.hpp"
#include "utility/ThreadPool.hpp"

// libglim
#include "glim/mdb.hpp"

// libstdc++
#include <string>
#include <iostream>

namespace verbatim {

class Database
{
    public:
        /* Methods/Member functions */
        Database(Traverse &t, utility::ThreadPool &tp);
        ~Database();

        void open(const std::string &path);
        void print_metrics(std::ostream &stream) const;
        size_t list_entries(std::ostream &stream) const;
    private:
        /* Forward declarations */
        struct Entry;

        /* Type definitions */
        class RegisterPath : public Traverse::Callback
        {
            public:
                RegisterPath(Database &d);
                void operator() (const Traverse::Path &p);
            private:
                Database &db;
        };

        /* Attributes/member variables */
        glim::Mdb *db;
        size_t entries, updates;

        Traverse &traverser;
        RegisterPath new_path;

        utility::ThreadPool &threads;

        /* Methods/Member functions (Entry) */
        bool lookup(Entry &e) const;
        void update(const Entry &e);

        /* Methods/Member functions (Path) */
        void update(const Traverse::Path &p);
};

} // verbatim

#endif

