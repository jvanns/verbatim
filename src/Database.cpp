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
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

// libc
#include <assert.h>

using std::string;
using std::ostream;

namespace {

struct GrabTag {
    GrabTag(glim::Mdb &d,
            const time_t f,
            const string &p) :
        db(d),
        modified(f),
        pathname(p)
    {
    }

    /*
     * Entry point for thread
     */
    void operator()()
    {
        verbatim::Tag v; // Value
        const string &k = pathname; // Key

        if (db.first(k, v) && v.modified == modified)
            return; // Assume entry exists and is untouched

        const TagLib::FileRef file(pathname.c_str(), false);
        const TagLib::Tag *tags = file.tag();

        if (!tags)
            return; // Not a valid audio file with tags?

        v.modified = modified;
        v.genre = tags->genre().to8Bit();
        v.album = tags->album().to8Bit();
        v.title = tags->title().to8Bit();
        v.artist = tags->artist().to8Bit();
        v.album_art_ref = add_image(tags, v);

        db.add(k, v);
    }

    size_t
    add_image(const TagLib::Tag *source,
              verbatim::Tag &destination)
    {
        static const verbatim::utility::Hash hasher;

        const TagLib::ID3v2::Tag *tag =
            dynamic_cast<const TagLib::ID3v2::Tag*>(source);

        if (!tag)
            return 0; /* Not an ID3v2 tag. Perhaps an OGG or FLAC? */

        const TagLib::ID3v2::FrameList &frames = tag->frameList("APIC");

        if (frames.isEmpty())
            return 0;

        /*
         * Adds only the first in the list and this assumes
         * it is the album cover, not the back or inside etc.
         */
        TagLib::ID3v2::AttachedPictureFrame *frame =
            static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());

        const char *data = frame->picture().data();
        const size_t len = frame->picture().size(),
                     hash = hasher(data, len);
        verbatim::Img album_art;

        assert(hash != 0);

        if (!db.first(hash, album_art)) {
            verbatim::Img::ByteVector::iterator i(album_art.data.begin());

            album_art.mimetype = frame->mimeType().to8Bit();
            album_art.data.reserve(len);
            copy(data, data + len, i);
            album_art.size = len;

            db.add(hash, album_art);
        }

        return hash;
    }

    glim::Mdb &db;
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
    db = new glim::Mdb(path.c_str(), 256, "verbatim", 0, false, 0600);
    assert(db != NULL);
}

size_t
Database::list_entries(ostream &stream) const
{
    assert(db != NULL); // open() must have been called first

    string key;
    verbatim::Tag value;
    size_t entry_count = 0;
    glim::Mdb::Iterator i(db->begin());
    const glim::Mdb::Iterator j(db->end());

    while (i != j) {
        i->getKey(key);
        i->getValue(value);

        stream <<
            key << '\t' <<
            value.modified << '\t' <<
            value.genre << '\t' <<
            value.artist << '\t' <<
            value.album << '\t' <<
            value.title << '\n';

        ++entry_count;
        ++i;
    }

    return entry_count;
}

void
Database::add_entry(const Entry &e)
{
}

void
Database::add_path(const Traverse::Path &p)
{
    assert(db != NULL); // open() must have been called first

    /*
     * Add or update a DB entry (a key-value pair)
     */
    if (S_ISREG(p.info->st_mode)) {
        const GrabTag gt(*db, p.info->st_mtime, p.name);
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

