/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Database.hpp"

using std::string;

namespace verbatim {

Database::Database(Traverse &t) : traverser(t), new_path(*this)
{
    traverser.register_callback(&new_path);
}

Database::~Database()
{
}

void
Database::open(const string &path)
{
    traverser.scan(path);
}

void
Database::add_path(const Traverse::Path &p)
{
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

