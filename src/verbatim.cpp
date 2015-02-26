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
        cerr << "Provide a source 'root' path" << endl;
        return 1;
    }

    Traverse t;
    Database db(t);

    db.store(argv[1]);

    return 0;
}

