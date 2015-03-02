/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// verbatim
#include "Database.hpp"

// libstdc++
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

namespace {

} // anonymous

int main(int argc, char *argv[])
{
    using verbatim::Traverse;
    using verbatim::Database;

    if (!argv[1]) {
        cerr << "Provide a filename in which to store data" << endl;
        return 1;
    }

    if (!argv[2]) {
        cerr << "Provide a source 'root' path" << endl;
        return 1;
    }

    Traverse t;
    Database db(t);

    db.open(argv[1]);
    t.scan(argv[2]);

    return 0;
}

