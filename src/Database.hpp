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
#include <vector>
#include <iostream>

namespace verbatim {

class Database
{
    public:
        /* Methods/Member functions */
        Database(Traverse &t, utility::ThreadPool &tp);
        ~Database();

        void open(const std::string &path);
        void update(const std::string &path);

        void aggregate_metrics();
        void print_metrics(std::ostream &stream) const;

        size_t list_entries(std::ostream &stream) const;
    private:
        /* Forward declarations */
        struct Entry;
        struct Accessor;
        struct VisitorBase;

        /* Type definitions */
        class RegisterPath : public Traverse::Callback
        {
            public:
                RegisterPath(Database &d);
                void operator() (const Traverse::Path &p);
            private:
                Database &db;
        };

        struct Metrics
        {
            size_t entries, updates;
            Metrics() : entries(0), updates(0) {}
        };

        /* Attributes/member variables */
        glim::Mdb *db;
        size_t entries, updates;

        double spread; // Approximation of distribution efficiency
        std::vector<Metrics> metrics; // Per-thread metrics

        Traverse &traverser;
        RegisterPath new_path;

        utility::ThreadPool &threads;

        /* Methods/Member functions */
        size_t mutable_visit(VisitorBase &v);
        size_t immutable_visit(VisitorBase &v) const;

        /* Methods/Member functions (Entry) */
        bool lookup(Entry &e) const;
        void update(const Entry &e);

        /* Methods/Member functions (Path) */
        void update(const Traverse::Path &p);

        /* Friend functions */
        template<typename Impl>
        friend class Visitor;
        template<typename Store, typename Key, typename Value>
        friend Entry retrieve(Store &s, Key &k, Value &v);
        friend std::ostream& operator<< (std::ostream &s, const Entry &e);
};

/*
 * Declarations of the above friends
 */
template<typename Store, typename Key, typename Value>
Database::Entry retrieve(Store &s, Key &k, Value &v);

std::ostream& operator<< (std::ostream &s, const Database::Entry &e);

} // verbatim

#endif

