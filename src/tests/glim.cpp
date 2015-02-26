/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// libglim
#include "mdb.hpp"

// libstdc++
#include <string>

// libc
#include <assert.h>

int main(int argc, char *argv[])
{
    {
        glim::Mdb db("/tmp/lmdb-test.db", 4, "lmdb-test", 0, true, 0600);
        std::string key("AKEY"), in("A Value"), out;

        db.add(key, in);
        assert(db.first(key, out));
        assert(in == out);
    }

    {
        glim::Mdb db("/tmp/lmdb-test.db", 4, "lmdb-test", 0, true, 0600);
        unsigned int key = 0xDEADBEEF, out;
        const unsigned int in = 0xB00B1E5;

        db.add(key, in);
        assert(db.first(key, out));
        assert(in == out);
    }

    return 0;
}

