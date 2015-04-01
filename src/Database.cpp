/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Database.hpp"

// verbatim
#include "Tag.hpp"
#include "utility/Hash.hpp"
#include "utility/Exception.hpp"

// Taglib
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

// boost serialization
#include <boost/serialization/list.hpp>

// libstdc++
#include <list>

// libc
#include <assert.h>

using std::endl;
using std::list;
using std::string;
using std::ostream;

using glim::Mdb;

namespace {

/*
 * Types private to this module
 */

enum TypeID
{
    NO_ID = 0,
    TAG_ID = 1,
    IMG_ID = 2
};

/*
 * Free functions private to this module
 */

bool
copy_img_tag_data(const TagLib::Tag *tags, verbatim::Img &img)
{
    const TagLib::ID3v2::Tag *id3 =
        dynamic_cast<const TagLib::ID3v2::Tag*>(tags);

    if (!id3)
        return false; // Not an ID3v2 tag. Perhaps an OGG or FLAC?

    const TagLib::ID3v2::FrameList &frames = id3->frameList("APIC");

    if (frames.isEmpty())
        return false;

    //
    // Adds only the first in the list and this assumes
    // it is the album cover, not the back or inside etc.
    //
    TagLib::ID3v2::AttachedPictureFrame *frame =
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());

    const char *data = frame->picture().data();
    const size_t len = frame->picture().size();
    verbatim::Img::ByteVector::iterator i(img.data.begin());

    if (len == 0)
        return false;

    img.mimetype = frame->mimeType().to8Bit();
    img.data.reserve(len);
    copy(data, data + len, i);
    img.size = len;

    return true;
}

} // anonymous

namespace verbatim {

/*
 * Interfaces
 */
 
struct Key
{
    /* Type definitions */
    typedef unsigned long long key_t;

    /* Methods/Member functions */
    Key();
    explicit Key(const string &s);
    explicit Key(const TagLib::Tag *tags);
    inline operator bool() const { return value != 0; }

    template<typename Archive>
    void
    serialize(Archive &archive, unsigned int /* version */);

    /* Attributes/member variables */
    key_t value;
    enum TypeID id;
    static const utility::Hash hasher;
};

class Value
{
    protected:
        /* Methods/Member functions */
        Value() {}
        virtual ~Value() {}
};

template<typename Type> class Adapter : public Value
{
    public:
        /* Methods/Member functions */
        explicit Adapter(Type &t) : impl(t) {}
        inline operator Type&() { return impl; }
        inline operator const Type&() const { return impl; }
    private:
        /* Attributes/member variables */
        Type &impl;
};

struct Database::Accessor
{
    /* Methods/Member functions */
    Accessor(Database &d, const char *p, const time_t f);

    /*
     * Entry point for thread
     */
    void operator()();

    /* Attributes/member variables */
    Database &db;
    const string pathname;
    const time_t modify_time;
};

struct Database::Entry
{
    /* Methods/Member functions */
    explicit Entry(const Key &k, Value &v);

    template<typename Archive>
    void
    serialize(Archive &archive, unsigned int /* version */);

    /* Attributes/member variables */
    list<Key> links; /* References to other linked DB entries */
    const Key &key;
    Value &value;
    bool modified;
};

/*
 * Implementations
 */

/*
 * verbatim::Key
 */
const utility::Hash Key::hasher = utility::Hash();

Key::Key() : value(0), id(NO_ID)
{
}

Key::Key(const string &s) : value(0), id(TAG_ID)
{
    value = hasher(s.c_str(), s.size());
}

Key::Key(const TagLib::Tag *tags) : value(0), id(IMG_ID)
{
    const TagLib::ID3v2::Tag *id3 =
        dynamic_cast<const TagLib::ID3v2::Tag*>(tags);

    if (!id3)
        return; // Not an ID3v2 tag. Perhaps an OGG or FLAC?

    const TagLib::ID3v2::FrameList &frames = id3->frameList("APIC");

    if (frames.isEmpty())
        return;

    //
    // Consider only the first in the list and this assumes
    // it is the album cover, not the back or inside etc.
    //
    TagLib::ID3v2::AttachedPictureFrame *frame =
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());

    const char *data = frame->picture().data();
    const size_t len = frame->picture().size();

    value =  hasher(data, len);
}

template<typename Archive>
void
Key::serialize(Archive &archive, unsigned int /* version */)
{
    archive & id & value;
}

ostream&
operator<< (ostream &s, const Key &k)
{
    return s << k.id << k.value;
}

/*
 * verbatim::Database::Accessor
 */
Database::Accessor::Accessor(Database &d, const char *p, const time_t f) :
    db(d),
    pathname(p),
    modify_time(f)
{
}

/*
 * Entry point for thread
 */
void
Database::Accessor::operator()()
{
    Tag tag;
    Adapter<Tag> tag_val(tag);
    const Key tag_key(pathname);
    Database::Entry tag_ent(tag_key, tag_val);

    if (!db.lookup(tag_ent) || tag.modified < modify_time) {
        const TagLib::FileRef file(pathname.c_str(), false);
        const TagLib::Tag *tags = file.tag();

        if (tags) {
            Img img;
            Adapter<Img> img_val(img);
            const Key img_key(tags);
            Database::Entry img_ent(img_key, img_val);

            if (!db.lookup(img_ent) && copy_img_tag_data(tags, img)) {
                img_ent.modified = true;
                db.update(img_ent);
            }

            tag.filename = pathname;
            tag.modified = modify_time;

            tag.genre = tags->genre().to8Bit();
            tag.album = tags->album().to8Bit();
            tag.title = tags->title().to8Bit();
            tag.artist = tags->artist().to8Bit();

            tag_ent.links.push_back(img_key);
            tag_ent.modified = true;
        }
    }

    db.update(tag_ent);
}

/*
 * verbatim::Database::Entry
 */
Database::Entry::Entry(const Key &k, Value &v) :
    key(k),
    value(v),
    modified(false)
{
}

template<typename Archive>
void
Database::Entry::serialize(Archive &archive, unsigned int /* version */)
{
    switch (key.id) {
        case NO_ID:
            throw utility::ValueError("Database::Entry::serialize",
                                      0,
                                      "Invalid ID (%d) in Key object",
                                      key.id);
            break;
        case TAG_ID:
            {
                Tag &tag = static_cast<Adapter<Tag>&>(value);
                archive & tag;
            }
            break;
        case IMG_ID:
            {
                Img &img = static_cast<Adapter<Img>&>(value);
                archive & img;
            }
            break;
    }

    archive & links;
}

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
 * Free functions
 */
template<>
Database::Entry
retrieve(const Database &db, Key &key, Adapter<Tag> &val)
{
    Database::Entry e(key, val);
    db.lookup(e);
    return e;
}

template<>
Database::Entry
retrieve(Mdb::Iterator &it, Key &key, Adapter<Tag> &val)
{
    Database::Entry e(key, val);
    it->getValue(e);
    assert(e.key);
    return e;
}

template<>
Database::Entry
retrieve(const Database &db, Key &key, Adapter<Img> &val)
{
    Database::Entry e(key, val);
    db.lookup(e);
    return e;
}

template<>
Database::Entry
retrieve(Mdb::Iterator &it, Key &key, Adapter<Img> &val)
{
    Database::Entry e(key, val);
    it->getValue(e);
    assert(e.key);
    return e;
}

ostream&
operator<< (ostream &s, const Database::Entry &e)
{
    switch (e.key.id) {
        case NO_ID:
            throw utility::ValueError("verbatim::operator<<",
                                      0,
                                      "Invalid ID (%d) in Key object",
                                      e.key.id);
            break;
        case TAG_ID:
            {
                Tag &tag = static_cast<Adapter<Tag>&>(e.value);
                s << e.key << '\t' << tag;
            }
            break;
        case IMG_ID:
            {
                Img &img = static_cast<Adapter<Img>&>(e.value);
                s << e.key << '\t' << img;
            }
            break;
    }

    return s;
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
    db = new Mdb(path.c_str(), 256, "verbatim", 0, false, 0600);
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

    list<Key> links;
    size_t entry_count = 0;

    for (Mdb::Iterator i = db->begin() ; i != db->end() ; ++i) {
        Key key;
        i->getKey(key);

        switch (key.id) {
            case NO_ID:
                throw utility::ValueError("Database::list_entries",
                                          0,
                                          "Invalid ID (%d) in Key object",
                                          key.id);
                break;
            case TAG_ID:
                {
                    Tag tag;
                    Adapter<Tag> val(tag);
                    Entry e(retrieve(i, key, val));

                    stream << e << endl;
                    links = e.links;
                }
                break;
            case IMG_ID:
                {
                    Img img;
                    Adapter<Img> val(img);
                    Entry e(retrieve(i, key, val));

                    stream << e << endl;
                    links = e.links;
                }
                break;
        }

        for (list<Key>::iterator j = links.begin() ; j != links.end() ; ++j) {
            switch (j->id) {
                case NO_ID:
                    throw utility::ValueError("Database::list_entries",
                                              0,
                                              "Invalid ID (%d) in Key object",
                                              j->id);
                    break;
                case TAG_ID:
                    {
                        Tag tag;
                        Adapter<Tag> val(tag);
                        Entry e(retrieve(*this, *j, val));

                        stream << "\n\t" << e << endl;
                    }
                    break;
                case IMG_ID:
                    {
                        Img img;
                        Adapter<Img> val(img);
                        Entry e(retrieve(*this, *j, val));

                        stream << "\n\t" << e << endl;
                    }
                    break;
            }
        }

        links.clear();
        ++entry_count;
    }

    return entry_count;
}

inline
bool
Database::lookup(Entry &e) const
{
    return db->first(e.key, e);
}

inline
void
Database::update(const Entry &e)
{
    if (e.modified) {
        assert(e.key);
        db->add(e.key, e);
    }

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
        const Accessor a(*this, p.name, p.info->st_mtime);
        threads.submit(a);
    }
}

} // verbatim

