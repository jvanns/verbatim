/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_DATABASE_HPP
#define VERBATIM_DATABASE_HPP

// verbatim
#include "Traverse.hpp"
#include "utility/ThreadPool.hpp"

// lmdb++
#include "lmdbxx/lmdb++.h"

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
    public:
        /* Forward declarations */
        class Transaction;
        template<typename Impl> class Visitor;
        template<typename Value> struct Entry;
    private:
        /* Forward declarations */
        struct Janitor; // For cleaning stale entries
        struct Maintainer; // For maintaining new and existing entries

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
            ssize_t lookups, added, removed, updated;
            Metrics() : lookups(0), added(0), removed(0), updated(0) {}
        };

        /* Attributes/member variables */
        lmdb::env db;

        double spread; // Approximation of distribution efficiency
        std::vector<Metrics> metrics; // Per-thread metrics

        Traverse &traverser;
        RegisterPath new_path;

        utility::ThreadPool &threads;

        /* Methods/Member functions */
        template<typename Impl> size_t visit(Visitor<Impl> &v);
        template<typename Impl> size_t visit(Visitor<Impl> &v) const;

        /* Methods/Member functions (Entry) */
        template<typename Value> bool lookup(Entry<Value> &e, Transaction &txn);
        template<typename Value> void update(const Entry<Value> &e, Transaction &txn);

        /* Methods/Member functions (Path) */
        void update(const Traverse::Path &p);

        /* Friend classes */
        template<typename Impl> friend class Visitor;
};

} // verbatim

#endif

