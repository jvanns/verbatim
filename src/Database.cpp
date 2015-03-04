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

    if (S_ISREG(p.info->st_mode)) {
        static const utility::Hash hasher;

        /*
         * Add or update a DB entry (a key-value pair)
         */
        Tag v;
        const size_t k = hasher(p.name, strlen(p.name));
        if (!db->first(k, v) || v.modified < p.info->st_mtime) {
            /*
             * TODO: Here is an ideal place to parallelise. Perhaps
             * introduce a work queue where worker threads can pull of
             * an item and execute the following. Only the db->add()
             * need be protected.
             */
            const bool add = v.modified == 0;
            const TagLib::FileRef file(p.name);
            const TagLib::Tag *tags = file.tag();

            if (!tags)
                return; // Not a valid auto file with tags?

            v.file = p.name;
            v.modified = p.info->st_mtime;
            v.genre = tags->genre().to8Bit();
            v.album = tags->album().to8Bit();
            v.title = tags->title().to8Bit();
            v.artist = tags->artist().to8Bit();

            if (add)
                db->add(k, v);
        }
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

