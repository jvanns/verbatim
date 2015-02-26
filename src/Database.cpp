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
Database::store(const string &path)
{
    traverser.scan(path);
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
}

} // verbatim

