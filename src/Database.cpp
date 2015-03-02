/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Database.hpp"

// libc
#include <assert.h>

using std::string;

namespace verbatim {

Database::Database(Traverse &t) : db(NULL), traverser(t), new_path(*this)
{
    traverser.register_callback(&new_path);
}

Database::~Database()
{
    if (db) {
        delete db;
        db = NULL;
    }
}

void
Database::open(const string &path)
{
    assert(db == NULL);
    db = new glim::Mdb(path.c_str(), 256, "verbatim", 0, true, 0600);
    assert(db != NULL);
}

void
Database::add_path(const Traverse::Path &p)
{
    assert(db != NULL); // open() must have been called first
}

Database::RegisterPath::RegisterPath(Database &db) : database(db)
{
}

Database::RegisterPath::~RegisterPath()
{
}

void
Database::RegisterPath::operator() (const Traverse::Path &p)
{
    database.add_path(p);
}

} // verbatim

