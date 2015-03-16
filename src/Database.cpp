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

using std::endl;
using std::string;
using std::ostream;

namespace verbatim {

/*
 * verbatim::Database::Entry
 */
struct Database::Entry
{
    Entry(Database &d, const time_t f, const char *p) :
        db(d),
        modified(0),
        pathname(p),
        modify_time(f)
    {
    }

    /*
     * Entry point for thread
     */
    void operator()()
    {
        if (!db.lookup(*this) || tag.modified < modify_time) {
            const TagLib::FileRef file(pathname.c_str(), false);
            const TagLib::Tag *tags = file.tag();

            if (tags) {
                tag.filename = pathname;
                tag.modified = modify_time;

                tag.genre = tags->genre().to8Bit();
                tag.album = tags->album().to8Bit();
                tag.title = tags->title().to8Bit();
                tag.artist = tags->artist().to8Bit();

                modified = 1;
            }
        }

        db.update(*this);
    }

    /*
    size_t
    add_image(const TagLib::Tag *source, Tag &destination)
    {
        static const utility::Hash hasher;

        const TagLib::ID3v2::Tag *tag =
            dynamic_cast<const TagLib::ID3v2::Tag*>(source);

        if (!tag)
            return 0; // Not an ID3v2 tag. Perhaps an OGG or FLAC?

        const TagLib::ID3v2::FrameList &frames = tag->frameList("APIC");

        if (frames.isEmpty())
            return 0;

        //
        // Adds only the first in the list and this assumes
        // it is the album cover, not the back or inside etc.
        //
        TagLib::ID3v2::AttachedPictureFrame *frame =
            static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());

        const char *data = frame->picture().data();
        const size_t len = frame->picture().size(),
                     hash = hasher(data, len);
        Img album_art;

        assert(hash != 0);

        if (!db.first(hash, album_art)) {
            Img::ByteVector::iterator i(album_art.data.begin());

            album_art.mimetype = frame->mimeType().to8Bit();
            album_art.data.reserve(len);
            copy(data, data + len, i);
            album_art.size = len;

            db.add(hash, album_art);
        }

        return hash;
    }
    */

    Tag tag;
    Database &db;
    size_t modified;

    const string pathname;
    const time_t modify_time;
};

/*
 * verbatim::Database::RegisterPath
 */
Database::RegisterPath::RegisterPath(Database &d) : db(d)
{
}

void
Database::RegisterPath::operator() (const Traverse::Path &p)
{
    db.update(p);
}
 
/*
 * verbatim::Database 
 */
Database::Database(Traverse &t, utility::ThreadPool &tp) :
    db(NULL),
    entries(0),
    updates(0),
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

void
Database::print_metrics(ostream &stream) const
{
    stream <<
        "verbatim[Database]: Total #entries = " <<
        entries <<
        endl <<
        "verbatim[Database]: Total #updates = " <<
        updates <<
        endl;
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
            value.filename << '\t' <<
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

inline
bool
Database::lookup(Entry &e) const
{
    return db->first(e.pathname, e.tag);
}

inline
void
Database::update(const Entry &e)
{
    if (e.modified)
        db->add(e.pathname, e.tag);

    /*
     * TODO: These may need to be protected by a mutex?
     */
    entries += 1;
    updates += e.modified;
}

inline
void
Database::update(const Traverse::Path &p)
{
    assert(db != NULL); // open() must have been called first

    /*
     * Add or update a DB entry (a key-value pair)
     */
    if (S_ISREG(p.info->st_mode)) {
        const Entry e(*this, p.info->st_mtime, p.name->c_str());
        threads.submit(e);
    }
}

} // verbatim

