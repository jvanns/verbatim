/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// verbatim
#include "utility/Timer.hpp"

// libglim
#include "mdb.hpp"

// boost
#include <boost/serialization/map.hpp>

// libstdc++
#include <map>
#include <string>
#include <iostream>

// libc
#include <assert.h>

int main(int argc, char *argv[])
{
    using std::cout;
    using std::cerr;
    using std::endl;

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

    static const int32_t max = 1UL << 20; // 1 million

    /*
     * Timed insertion of a trivial type
     */
    {
        size_t time = 0;
        verbatim::Timer t;
        glim::Mdb db("/tmp/lmdb-test.db", 128, "lmdb-test", 0, true, 0600);

        cerr << "Insert of " << max << " TT: ";
        t.start();
        for (int32_t i = 0, j = max ; j > 0 ; ++i, --j)
            db.add(i, j);
        time = t.elapsed();
        cerr << time << " ns\n";
    }

    /*
     * Timed retrieval of a trivial type
     */
    {
        size_t time = 0;
        verbatim::Timer t;
        glim::Mdb db("/tmp/lmdb-test.db", 128, "lmdb-test", 0, true, 0600);

        cerr << "Retrieval of " << max << " TT: ";
        t.start();
        for (int32_t i = 0, j = max ; j > 0 ; ++i, --j) {
            int32_t out;
            db.first(i, out);
            assert(out == j);
        }

        time = t.elapsed();
        cerr << time << " ns\n";
    }

    /*
     * Timed insertion of a complex type
     */
    {
        size_t time = 0;
        verbatim::Timer t;
        glim::Mdb db("/tmp/lmdb-test.db", 128, "lmdb-test", 0, true, 0600);

        cerr << "Insert of " << max << " CT: ";
        for (int32_t i = 0, j = max ; j > 0 ; ++i, --j) {
            std::map<int32_t, int32_t> m;
            const uint64_t key = ((uint64_t)i << 32) | (uint64_t)j;

            m[i] = j;
            t.start();
            db.add(key, m);
            time += t.elapsed();
        }
        cerr << time << " ns\n";
    }

    /*
     * Timed retrieval of a complex type
     */
    {
        size_t time = 0;
        verbatim::Timer t;
        glim::Mdb db("/tmp/lmdb-test.db", 128, "lmdb-test", 0, true, 0600);

        cerr << "Retrieval of " << max << " CT: ";
        for (int32_t i = 0, j = max ; j > 0 ; ++i, --j) {
            std::map<int32_t, int32_t> m;
            const uint64_t key = ((uint64_t)i << 32) | (uint64_t)j;

            m[i] = j;
            t.start();
            db.first(key, m);
            time += t.elapsed();
            assert(m.begin()->first == i &&
                   m.begin()->second == j);
        }
        cerr << time << " ns\n";
    }

    return 0;
}

