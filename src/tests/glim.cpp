/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// libglim
#include "mdb.hpp"

// boost
#include <boost/serialization/map.hpp>

// libstdc++
#include <map>
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

    /*
     * Timed insertion of a trivial type
     */
    {
        glim::Mdb db("/tmp/lmdb-test.db", 128, "lmdb-test", 0, true, 0600);

        for (int32_t i = 0, j = 1UL << 20 ; j > 0 ; ++i, --j)
            db.add(i, j);
    }

    /*
     * Timed insertion of a complex type
     */
    {
        glim::Mdb db("/tmp/lmdb-test.db", 128, "lmdb-test", 0, true, 0600);

        for (int32_t i = 0, j = 1UL << 20 ; j > 0 ; ++i, --j) {
            std::map<int32_t, int32_t> m;
            const uint64_t key = ((uint64_t)i << 32) | (uint64_t)j;

            m[i] = j;
            db.add(key, m);
        }
    }

    return 0;
}

