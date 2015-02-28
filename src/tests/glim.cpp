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
    /*
     * Test std::string KVP insert and retrieval
     */
    {
        glim::Mdb db("/tmp/lmdb-test.db", 4, "lmdb-test", 0, true, 0600);
        std::string key("AKEY"), in("A Value"), out;

        db.add(key, in);
        assert(db.first(key, out));
        assert(in == out);
    }

    /*
     * Test integer KVP insert and retrieval
     */
    {
        glim::Mdb db("/tmp/lmdb-test.db", 4, "lmdb-test", 0, true, 0600);
        unsigned int key = 0xDEADBEEF, out;
        const unsigned int in = 0xB00B1E5;

        db.add(key, in);
        assert(db.first(key, out));
        assert(in == out);
    }

    /*
     * Test std::string retrieval (disk persistance)
     */
    {
        glim::Mdb db("/tmp/lmdb-test.db", 4, "lmdb-test", 0, true, 0600);
        std::string key("AKEY"), expected("A Value"), value;

        assert(db.first(key, value));
        assert(value == expected);
    }

    /*
     * Test key removal
     */
    {
        glim::Mdb db("/tmp/lmdb-test.db", 4, "lmdb-test", 0, true, 0600);
        const unsigned int key = 0xDEADBEEF;

        assert(db.erase(key));
    }

    /*
     * Test key absence (disk persistance)
     */
    {
        glim::Mdb db("/tmp/lmdb-test.db", 4, "lmdb-test", 0, true, 0600);
        unsigned int key = 0xDEADBEEF, out;

        assert(!db.first(key, out));
    }

    return 0;
}

