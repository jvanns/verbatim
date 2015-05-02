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
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>

// boost serialization
#include <boost/serialization/list.hpp>

// libstdc++
#include <list>

// libc
#include <math.h>
#include <assert.h>
#include <unistd.h>

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
copy_img_tag_data(const TagLib::ID3v2::Tag *tag, verbatim::Img &img)
{
    const TagLib::ID3v2::FrameList &frames = tag->frameList("APIC");

    if (frames.isEmpty())
        return false;

    /*
     * Adds only the first in the list and this assumes
     * it is the album cover, not the back or inside etc.
     */
    TagLib::ID3v2::AttachedPictureFrame *frame =
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());

    const TagLib::ByteVector bv(frame->picture());

    if (bv.size() == 0)
        return false;

    img.data.reserve(bv.size());
    img.data.insert(img.data.begin(), bv.data(), bv.data() + bv.size());

    img.size = bv.size();
    img.mimetype = frame->mimeType().to8Bit();

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
    explicit Key(const TagLib::ID3v2::Tag *tag);
    inline operator bool() const { return id != NO_ID && value != 0; }

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

    size_t added,
           removed,
           updated;
};

struct Database::Updater
{
    /* Methods/Member functions */
    Updater(Database &d, const char *p, const time_t f);

    /*
     * Entry point for thread
     */
    void operator()();

    /* Attributes/member variables */
    Database &db;
    const string path;
    const time_t modify_time;
};

struct Database::Remover
{
    /* Methods/Member functions */
    Remover(Database &d);

    /*
     * Entry point for thread
     */
    void operator()();

    /* Attributes/member variables */
    Database &db;
};

struct Database::VisitorBase
{
    virtual ~VisitorBase() {}
    virtual void operator() (Database::Entry &e) = 0;
};

/*
 * The friends
 */
template<typename Impl> class Visitor;
template<typename Store, typename Key, typename Value>
Database::Entry retrieve(Store &s, Key &k, Value &v);
std::ostream& operator<< (std::ostream &s, const Database::Entry &e);

template<typename Impl>
class Visitor : public Database::VisitorBase
{
    public:
        /* Methods/Member functions */
        Visitor() : entry(NULL) {}
        virtual ~Visitor() {}
        virtual void operator() () = 0;

        inline void operator() (Database::Entry &e)
        {
            Impl &impl = static_cast<Impl&>(*this);
            entry = &e;
            impl();
        }
    private:
        /* Attributes/member variables */
        Database::Entry *entry;

        friend Impl;
};

struct Printer : public Visitor<Printer>
{
    /* Methods/Member functions */
    Printer(ostream &s) : stream(s) {}
    virtual ~Printer() {}

    void operator() ()
    {
        stream << *entry << endl;
    }

    /* Attributes/member variables */
    ostream &stream;
};

struct Verifier : public Visitor<Verifier>
{
    /* Methods/Member functions */
    Verifier(Database &d) : db(d) {}
    virtual ~Verifier() {}

    void operator() ()
    {
        if (entry->key.id != TAG_ID)
            return;

        const Tag &tag = static_cast<Adapter<Tag>&>(entry->value);
        if (access(tag.filename.c_str(), F_OK) == 0)
            return;

        /*
         * FIXME: Remove any Img entry too and any other entry
         * that references it.
         */

        /*
         * If the file doesn't exist anymore, remove the entry
         */
        entry->removed = 1;
        db.update(*entry);
    }

    /* Attributes/member variables */
    Database &db;
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

Key::Key(const string &s) : value(0), id(NO_ID)
{
    value = hasher(s.c_str(), s.size());
    id = TAG_ID;
}

Key::Key(const TagLib::ID3v2::Tag *tag) : value(0), id(NO_ID)
{
    const TagLib::ID3v2::FrameList &frames = tag->frameList("APIC");

    if (frames.isEmpty())
        return;

    /*
     * Adds only the first in the list and this assumes
     * it is the album cover, not the back or inside etc.
     */
    TagLib::ID3v2::AttachedPictureFrame *frame =
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());

    const TagLib::ByteVector bv(frame->picture());

    if (bv.size() == 0)
        return;

    value = hasher(bv.data(), bv.size());
    id = IMG_ID;
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
 * verbatim::Database::Entry
 */
Database::Entry::Entry(const Key &k, Value &v) :
    key(k),
    value(v),
    added(0),
    removed(0),
    updated(0)
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
 * verbatim::Database::Updater
 */
Database::Updater::Updater(Database &d, const char *p, const time_t f) :
    db(d),
    path(p),
    modify_time(f)
{
}

/*
 * Entry point for thread
 */
void
Database::Updater::operator()()
{
    TagLib::MPEG::File f(path.c_str());

    if (!(f.isValid() && f.hasID3v2Tag()))
        return;

    Tag tag;
    const Key tag_key(path);
    Adapter<Tag> tag_ref(tag);
    Database::Entry tag_ent(tag_key, tag_ref);

    if (!db.lookup(tag_ent) || tag.modified < modify_time) {
        const TagLib::ID3v2::Tag *tags = f.ID3v2Tag();
        const Key img_key(tags);

        if (img_key) {
            Img img;
            Adapter<Img> img_ref(img);
            Database::Entry img_ent(img_key, img_ref);

            img_ent.added = (!db.lookup(img_ent) &&
                              copy_img_tag_data(tags, img));
            tag_ent.links.push_back(img_key);
            db.update(img_ent);
        }

        tag_ent.added = tag.modified == 0;
        tag_ent.updated = !tag_ent.added;

        tag.filename = path;
        tag.modified = modify_time;
        tag.genre = tags->genre().to8Bit();
        tag.album = tags->album().to8Bit();
        tag.title = tags->title().to8Bit();
        tag.artist = tags->artist().to8Bit();
    }

    db.update(tag_ent);
}

/*
 * verbatim::Database::Remover
 */
Database::Remover::Remover(Database &d) : db(d)
{
}

/*
 * Entry point for thread
 */
void
Database::Remover::operator()()
{
    Verifier v(db);
    db.mutable_visit(v);
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
    spread(0.0),
    metrics(tp.size() + 1),
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
Database::update(const string &path)
{
    const Remover r(*this);
    open(path);
    threads.submit(r);
}

void
Database::print_metrics(ostream &stream) const
{
    stream <<
        "verbatim[Database]: Total #added = " <<
        metrics[0].added <<
        endl <<
        "verbatim[Database]: Total #removed = " <<
        metrics[0].removed <<
        endl <<
        "verbatim[Database]: Total #updated = " <<
        metrics[0].updated <<
        endl <<
        "verbatim[Database]: Total #entries = " <<
        metrics[0].entries <<
        endl <<
        "verbatim[Database]: Spread measure = ~" <<
        spread <<
        "%\n";
}

size_t
Database::list_entries(ostream &stream) const
{
    Printer p(stream);
    return immutable_visit(p);
}

/*
 * Sum all of the per-thread metrics
 */
void
Database::aggregate_metrics()
{
    Metrics &aggregate = metrics[0];

    for (size_t i = 1 ; i < metrics.size() ; ++i) {
        aggregate.added += metrics[i].added;
        aggregate.removed += metrics[i].removed;
        aggregate.updated += metrics[i].updated;
        aggregate.entries += metrics[i].entries;
    }

    /*
     * WUPT: Work Units Per Thread (ideal mean of)
     *
     * Using the target or ideal spread of queue items per worker thread (wupt)
     * and the standard deviation (measure of variance of items per thread),
     * display a rough measure of queue efficiency. Calculate and store it as
     * the relative standard deviation (a percentage) for easier reading. Note
     * that the value displayed is in fact the wupt minus the standard
     * deviation so the higher the percentage the more efficient, or better
     * spread, the work load is between the threads. The lower this number the
     * less efficient verbatim has been - some threads did little while others
     * did a lot and therefore not as many parallel tasks were issued as there
     * could have been.
     */

    double n = metrics.size() - 1,
           wupt = metrics[0].entries / n, x = 0.0f, y = 0.0f;
    for (size_t i = 1 ; i < metrics.size() ; ++i) {
        y = metrics[i].entries - wupt;
        x += (y * y);
    }

    x = wupt - sqrt(x / n);
    spread = fabs(x / wupt) * 100.0f;
}

size_t
Database::mutable_visit(VisitorBase &v)
{
    assert(db != NULL); // open() must have been called first

    list<Key> links;
    size_t visits = 0;
    const Database &self = *this;

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

                v(e);
                visits++;
                links = e.links;
            }
            break;
        case IMG_ID:
            {
                Img img;
                Adapter<Img> val(img);
                Entry e(retrieve(i, key, val));

                v(e);
                visits++;
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
                    Entry e(retrieve(self, *j, val));

                    v(e);
                    visits++;
                }
                break;
            case IMG_ID:
                {
                    Img img;
                    Adapter<Img> val(img);
                    Entry e(retrieve(self, *j, val));

                    v(e);
                    visits++;
                }
                break;
            }
        }

        links.clear();
    }

    return visits;
}

size_t
Database::immutable_visit(VisitorBase &v) const
{
    assert(db != NULL); // open() must have been called first

    list<Key> links;
    size_t visits = 0;
    const Database &self = *this;

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

                v(e);
                visits++;
                links = e.links;
            }
            break;
        case IMG_ID:
            {
                Img img;
                Adapter<Img> val(img);
                Entry e(retrieve(i, key, val));

                v(e);
                visits++;
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
                    Entry e(retrieve(self, *j, val));

                    v(e);
                    visits++;
                }
                break;
            case IMG_ID:
                {
                    Img img;
                    Adapter<Img> val(img);
                    Entry e(retrieve(self, *j, val));

                    v(e);
                    visits++;
                }
                break;
            }
        }

        links.clear();
    }

    return visits;
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
    Metrics &m = metrics[threads.index() + 1]; // Metrics of current thread

    assert(e.key);
    if (e.added || e.updated) {
        db->add(e.key, e);
        m.entries++;
    } else if (e.removed) {
        db->erase(e.key);
        m.entries--;
    }

    m.added += e.added;
    m.removed += e.removed;
    m.updated += e.updated;
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
        const Updater u(*this, p.name, p.info->st_mtime);
        threads.submit(u);
    }
}

} // verbatim

