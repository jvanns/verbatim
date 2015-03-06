/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Database.hpp"

// verbatim
#include "Tag.hpp"
#include "utility/Hash.hpp"

// Taglib
#include <taglib/tag.h>
#include <taglib/fileref.h>

// libc
#include <assert.h>

using std::string;

namespace {

struct GrabTag {
    GrabTag(glim::Mdb &d,
            boost::mutex &m,
            const time_t f,
            const string p) :
        db(d),
        mutex(m),
        modified(f),
        pathname(p)
    {
    }

    void operator()()
    {
        verbatim::Tag v;
        const char *filename = pathname.c_str();
        static const verbatim::utility::Hash hasher;
        const size_t k = hasher(filename, pathname.size());

        if (db.first(k, v) && v.modified >= modified)
            return; // Assume entry exists and is untouched

        const bool add = v.modified == 0;
        const TagLib::FileRef file(filename);
        const TagLib::Tag *tags = file.tag();

        if (!tags)
            return; // Not a valid auto file with tags?

        v.file = pathname;
        v.modified = modified;
        v.genre = tags->genre().to8Bit();
        v.album = tags->album().to8Bit();
        v.title = tags->title().to8Bit();
        v.artist = tags->artist().to8Bit();

        if (add) {
            /*
             * TODO: Investigate whether the lock guard is really needed
             * here. It looks, at a glance, as though the underlying LMDB
             * database is thread-safe (MDB C, not the GLIM C++ wrapper).
             */
            boost::lock_guard<boost::mutex> guard(mutex);
            db.add(k, v);
        }
    }

    glim::Mdb &db;
    boost::mutex &mutex;
    const time_t modified;
    const string pathname;
};

} // anonymous

namespace verbatim {
 
Database::Database(Traverse &t, utility::ThreadPool &tp) :
    db(NULL),
    traverser(t),
    new_path(*this),
    threads(tp)
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

    /*
     * Add or update a DB entry (a key-value pair)
     */
    if (S_ISREG(p.info->st_mode)) {
        GrabTag gt(*db, mutex, p.info->st_mtime, p.name);
        threads.submit(gt);
    }
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

