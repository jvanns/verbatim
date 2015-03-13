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
        size_t list_entries(std::ostream &stream) const;
    private:
        /* Forward declarations */
        struct Entry;

        /* Type definitions */
        class RegisterPath : public Traverse::Callback
        {
            public:
                RegisterPath(Database &db);
                ~RegisterPath();

                void operator() (const Traverse::Path &p);
            private:
                Database &database;
        };

        /* Attributes/member variables */
        glim::Mdb *db;
        Traverse &traverser;
        RegisterPath new_path;
        utility::ThreadPool &threads;

        /* Methods/Member functions */
        void add_entry(const Entry &e);
        void add_path(const Traverse::Path &p);

        /* Friend class declarations */
        friend struct Entry;
        friend class RegisterPath;
};

} // verbatim

#endif

