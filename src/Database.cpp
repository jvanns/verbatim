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
#include <boost/serialization/set.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// libstdc++
#include <set>
#include <vector>
#include <sstream>

// libc
#include <math.h>
#include <assert.h>
#include <unistd.h>

// STL
using std::set;
using std::endl;
using std::string;
using std::vector;
using std::ostream;
using std::istringstream;
using std::ostringstream;

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
copy_img_data(const TagLib::ID3v2::Tag *tag, verbatim::Img &img)
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

static const std::ios_base::openmode BINARY_STREAM = std::ios_base::in | \
                                                     std::ios_base::out | \
                                                     std::ios_base::binary;

template<typename Type>
Type
reconstruct(const lmdb::val &serialised_data)
{
    Type value;
    istringstream stream(BINARY_STREAM);
    char *buf = const_cast<char*>(serialised_data.data());
    boost::archive::binary_iarchive ba(stream, boost::archive::no_header);

    stream.rdbuf()->pubsetbuf(buf, serialised_data.size());

    ba >> value;
    if (stream.fail())
        throw verbatim::utility::StreamError("reconstruct()",
                                             errno,
                                             "Failed to serialise data");

    return value;
}

template<typename Type>
string
deconstruct(const Type &value)
{
    ostringstream stream(BINARY_STREAM);
    boost::archive::binary_oarchive ba(stream, boost::archive::no_header);

    ba << value;
    if (stream.fail())
        throw verbatim::utility::StreamError("deconstruct()",
                                             errno,
                                             "Failed to serialise data");

    return stream.str();
}

} // anonymous

namespace verbatim {

/*
 * Key (interface)
 */
struct Key
{
    /* Type definitions */
    typedef unsigned long long key_t;

    /* Methods/Member functions */
    Key();
    explicit Key(const string &s);
    explicit Key(const TagLib::ID3v2::Tag *tag);

    operator bool() const;
    bool operator== (const Key &other) const;

    template<typename Archive>
    void
    serialize(Archive &archive, unsigned int /* version */);

    /* Attributes/member variables */
    key_t value;
    enum TypeID id;
    static const utility::Hash hasher;
};

/*
 * Key (implementation)
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

inline
Key::operator bool() const
{
    return id != NO_ID && value != 0;
}

inline
bool
Key::operator== (const Key &other) const
{
    return id == other.id && value == other.value;
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
 * Transaction (interface)
 */
class Database::Transaction
{
    public:
        /* Methods/Member functions */
        Transaction(const Transaction &tn);
        Transaction(const Database &db);

        void commit();
        lmdb::cursor cur();
        MDB_stat stats() const;

        void del(lmdb::val &key);
        void put(lmdb::val &key, lmdb::val &val);
        bool get(lmdb::val &key, lmdb::val &val);
    private:
        /* Attributes/member variables */
        lmdb::txn txn;
        lmdb::dbi dbi;
        const lmdb::env &env;
};

/*
 * Transaction (implementation)
 */
Database::Transaction::Transaction(const Transaction &tn) :
    txn(lmdb::txn::begin(tn.env)),
    dbi(lmdb::dbi::open(txn)),
    env(tn.env)
{
}

Database::Transaction::Transaction(const Database &db) :
    txn(lmdb::txn::begin(db.lmdb_env)),
    dbi(lmdb::dbi::open(txn)),
    env(db.lmdb_env)
{
}

inline
void
Database::Transaction::commit()
{
    txn.commit();
}

inline
lmdb::cursor
Database::Transaction::cur()
{
    return lmdb::cursor::open(txn, dbi);
}

inline
MDB_stat
Database::Transaction::stats() const
{
    return dbi.stat(txn);
}

inline
void
Database::Transaction::del(lmdb::val &key)
{
    dbi.del(txn, key);
}

inline
void
Database::Transaction::put(lmdb::val &key, lmdb::val &val)
{
    dbi.put(txn, key, val);
}

inline
bool
Database::Transaction::get(lmdb::val &key, lmdb::val &val)
{
    return dbi.get(txn, key, val);
}

/*
 * Entry (interface)
 */
template<typename Value> struct Database::Entry
{
    /* Methods/Member functions */
    Entry();
    explicit Entry(const Key &k);
    explicit Entry(const Key &k, const Value &v);

    template<typename Archive>
    void
    serialize(Archive &archive, unsigned int /* version */);

    /* Attributes/member variables */
    Key key;
    Value value;
    set<Key> links_to,   /* References to other linked DB entries */
             links_from; /* References from other linked DB entries */

    size_t added,
           removed,
           updated;
};

/*
 * Entry (implementation)
 */
template<typename Value>
Database::Entry<Value>::Entry() : added(0), removed(0), updated(0)
{
}

template<typename Value>
Database::Entry<Value>::Entry(const Key &k) :
    key(k),
    added(0),
    removed(0),
    updated(0)
{
}

template<typename Value>
Database::Entry<Value>::Entry(const Key &k, const Value &v) :
    key(k),
    value(v),
    added(0),
    removed(0),
    updated(0)
{
}

template<typename Value>
template<typename Archive>
void
Database::Entry<Value>::serialize(Archive &archive, unsigned int /* version */)
{
    archive & key & value & links_to & links_from;
}

/*
 * Visitor
 */
template<typename Impl> class Database::Visitor
{
    public:
        /* Methods/Member functions */
        template<typename T>
        inline
        void operator() (Database::Entry<T> &e, Database::Transaction &t)
        {
            Impl &impl = static_cast<Impl&>(*this);
            impl(e, t);
        }

        template<typename T>
        inline
        void operator() (const Database::Entry<T> &e, Database::Transaction &t)
        {
            Impl &impl = static_cast<Impl&>(*this);
            impl(e, t);
        }
};

/*
 * Printer (Visitor implementor)
 */
struct Printer : public Database::Visitor<Printer>
{
    /* Methods/Member functions */
    Printer(ostream &s) : stream(s) {}
    virtual ~Printer() {}

    template<typename T>
    inline
    void operator() (const Database::Entry<T> &e, Database::Transaction &t)
    {
        assert(e.key.id != NO_ID);
        stream << e.key << '\t' << e.value << endl;

        set<Key>::iterator i(e.links_to.begin()), j(e.links_to.end());
        while (i != j) {
            stream << "\t[->]\t" << *i;
            ++i;
        }

        if (e.links_to.size() > 0)
            stream << endl;

        for (i = e.links_from.begin(), j = e.links_from.end() ; i != j ; ++i)
            stream << "\t[<-]\t" << *i;

        if (e.links_from.size() > 0)
            stream << endl;
    }

    /* Attributes/member variables */
    ostream &stream;
};

/*
 * Janitor (Visitor implementor)
 */
struct Database::Janitor : public Database::Visitor<Janitor>
{
    /* Methods/Member functions */
    Janitor(Database &d) : db(d) {}
    virtual ~Janitor() {}

    void operator()(); // THREAD ENTRY POINT

    template<typename T>
    void operator() (Database::Entry<T> &e, Database::Transaction &t);

    /* Attributes/member variables */
    Database &db;
};

void
Database::Janitor::operator()() // THREAD ENTRY POINT
{
    db.visit<Janitor>(*this);
}

template<>
inline
void
Database::Janitor::operator()<Img> (Database::Entry<Img> &e,
                                    Database::Transaction &t)
{
}

template<>
inline
void
Database::Janitor::operator()<Tag> (Database::Entry<Tag> &e,
                                    Database::Transaction &t)
{
    assert(e.key.id == TAG_ID);

    if (access(e.value.filename.c_str(), F_OK) == 0)
        return;

    Transaction txn(t);
    set<Key>::iterator i(e.links_from.begin()), j(e.links_from.end());
    while (i != j) {
        switch (i->id) {
        case NO_ID:
            throw utility::ValueError("Janitor::operator()",
                                      0,
                                      "Invalid ID (%d) in Key object",
                                      *i);
        case TAG_ID: {
                Entry<Tag> link(*i);
                if (db.lookup(link, txn)) {
                    link.links_to.erase(e.key);
                    link.updated = 1;
                    db.update(link, txn);
                }
            }
            break;
        case IMG_ID: {
                Entry<Img> link(*i);
                if (db.lookup(link, txn)) {
                    link.links_to.erase(e.key);
                    link.updated = 1;
                    db.update(link, txn);
                }
            }
            break;
        }
        ++i;
    }

    for (i = e.links_to.begin(), j = e.links_to.end() ; i != j ; ++i) {
        switch (i->id) {
        case NO_ID:
            throw utility::ValueError("Janitor::operator()",
                                      0,
                                      "Invalid ID (%d) in Key object",
                                      *i);
        case TAG_ID: {
                Entry<Tag> link(*i);
                if (db.lookup(link, txn)) {
                    link.links_from.erase(e.key);
                    link.updated = 1;
                    db.update(link, txn);
                }
            }
            break;
        case IMG_ID: {
                Entry<Img> link(*i);
                if (db.lookup(link, txn)) {
                    link.links_from.erase(e.key);
                    link.updated = 1;
                    db.update(link, txn);
                }
            }
            break;
        }
    }

    /*
     * If the file doesn't exist anymore, remove the entry
     */
    e.removed = 1;
    db.update(e, txn);

    txn.commit();
}

/*
 * Maintainer (interface)
 */
struct Database::Maintainer
{
    /* Methods/Member functions */
    Maintainer(Database &d, const char *p, const time_t f);

    void operator()(); // THREAD ENTRY POINT

    /* Attributes/member variables */
    Database &db;
    const string path;
    const time_t modify_time;
};

/*
 * Maintainer (implementation)
 */
Database::Maintainer::Maintainer(Database &d, const char *p, const time_t f) :
    db(d),
    path(p),
    modify_time(f)
{
}

void
Database::Maintainer::operator()() // THREAD ENTRY POINT
{
    TagLib::MPEG::File f(path.c_str());

    if (!(f.isValid() && f.hasID3v2Tag()))
        return;

    const Key tag_key(path);
    Database::Transaction txn(db);
    Database::Entry<Tag> tag_ent(tag_key);

    if (!db.lookup<Tag>(tag_ent, txn) ||
        tag_ent.value.modified < modify_time)
    {
        const TagLib::ID3v2::Tag *tags = f.ID3v2Tag();
        const Key img_key(tags);

        if (img_key) {
            Database::Entry<Img> img_ent(img_key);

            if (!db.lookup<Img>(img_ent, txn) &&
                copy_img_data(tags, img_ent.value))
                img_ent.added = 1;

            img_ent.links_from.insert(tag_key);
            img_ent.updated = !img_ent.added;
            tag_ent.links_to.insert(img_key);
            db.update<Img>(img_ent, txn);
        }

        tag_ent.added = tag_ent.value.modified == 0;
        tag_ent.updated = !tag_ent.added;

        tag_ent.value.filename = path;
        tag_ent.value.modified = modify_time;
        tag_ent.value.genre = tags->genre().to8Bit();
        tag_ent.value.album = tags->album().to8Bit();
        tag_ent.value.title = tags->title().to8Bit();
        tag_ent.value.artist = tags->artist().to8Bit();

        db.update<Tag>(tag_ent, txn);
    }

    txn.commit();
}

/*
 * RegisterPath (implementation)
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
 * Database  (implementation)
 */
Database::Database(Traverse &t, utility::ThreadPool &tp) :
    lmdb_env(lmdb::env::create()),
    spread(0.0),
    metrics(tp.size() + 1),
    traverser(t),
    new_path(*this),
    threads(tp)
{
    lmdb_env.set_mapsize((1024 * 1024) * 64); // 64MB
    traverser.register_callback(&new_path);
}

Database::~Database()
{
}

void
Database::open(const string &path)
{
    lmdb_env.open(path.c_str(), 0, 0600);
}

void
Database::update(const string &path)
{
    const Janitor j(*this);
    open(path);
    threads.submit(j);
}

void
Database::print_metrics(ostream &stream) const
{
    Transaction txn(*this);
    const MDB_stat db_stats(txn.stats());

    stream <<
        "verbatim[Database]: Tree depth =     " <<
        db_stats.ms_depth <<
        endl <<
        "verbatim[Database]: #branches =      " <<
        db_stats.ms_branch_pages <<
        endl <<
        "verbatim[Database]: #leaves =        " <<
        db_stats.ms_leaf_pages <<
        endl <<
        "verbatim[Database]: Total #added =   " <<
        metrics[0].added <<
        endl <<
        "verbatim[Database]: Total #removed = " <<
        metrics[0].removed <<
        endl <<
        "verbatim[Database]: Total #updated = " <<
        metrics[0].updated <<
        endl <<
        "verbatim[Database]: Total #lookups = " <<
        metrics[0].lookups <<
        endl <<
        "verbatim[Database]: Total #entries = " <<
        db_stats.ms_entries <<
        endl <<
        "verbatim[Database]: Spread measure = ~" <<
        spread <<
        "%\n";
}

size_t
Database::list_entries(ostream &stream) const
{
    Printer p(stream);
    return visit<Printer>(p);
}

/*
 * Sum all of the per-thread metrics
 */
void
Database::aggregate_metrics()
{
    Metrics &aggregate = metrics[0];
    vector<double> activity(metrics.size(), 0.0);

    for (size_t i = 1 ; i < metrics.size() ; ++i) {
        aggregate.added += metrics[i].added;
        aggregate.removed += metrics[i].removed;
        aggregate.updated += metrics[i].updated;
        aggregate.lookups += metrics[i].lookups;

        activity[i] =
            metrics[i].added +
            metrics[i].removed +
            metrics[i].updated +
            metrics[i].lookups;
        activity[0] += activity[i];
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

    double n = activity.size() - 1,
           wupt = activity[0] / n, x = 0.0f, y = 0.0f;
    for (size_t i = 1 ; i < activity.size() ; ++i) {
        y = activity[i] - wupt;
        x += (y * y);
    }

    x = wupt - sqrt(x / n);
    spread = fabs(x / wupt) * 100.0f;
}

template<typename Impl>
size_t
Database::visit(Visitor<Impl> &v)
{
    size_t visits = 0;
    Transaction txn(*this);
    lmdb::cursor cur(txn.cur());
    lmdb::val lmdb_key, lmdb_val;

    while (cur.get(lmdb_key, lmdb_val, MDB_NEXT)) {
        const Key key(reconstruct<Key>(lmdb_key));

        switch (key.id) {
        case NO_ID:
            throw utility::ValueError("Database::visit",
                                      0,
                                      "Invalid ID (%d) in Key object",
                                      key.id);
        case TAG_ID: {
                Entry<Tag> e(reconstruct<Entry<Tag> >(lmdb_val));
                v(e, txn);
            }
            break;
        case IMG_ID: {
                Entry<Img> e(reconstruct<Entry<Img> >(lmdb_val));
                v(e, txn);
            }
            break;
        }

        ++visits;
    }

    return visits;
}

template<typename Impl>
size_t
Database::visit(Visitor<Impl> &v) const
{
    size_t visits = 0;
    Transaction txn(*this);
    lmdb::cursor cur(txn.cur());
    lmdb::val lmdb_key, lmdb_val;

    while (cur.get(lmdb_key, lmdb_val, MDB_NEXT)) {
        const Key key(reconstruct<Key>(lmdb_key));

        switch (key.id) {
        case NO_ID:
            throw utility::ValueError("Database::visit",
                                      0,
                                      "Invalid ID (%d) in Key object",
                                      key.id);
        case TAG_ID: {
                const Entry<Tag> e(reconstruct<Entry<Tag> >(lmdb_val));
                v(e, txn);
            }
            break;
        case IMG_ID: {
                const Entry<Img> e(reconstruct<Entry<Img> >(lmdb_val));
                v(e, txn);
            }
            break;
        }

        ++visits;
    }

    return visits;
}

template<typename Value>
inline
bool
Database::lookup(Entry<Value> &e, Transaction &txn)
{
    const string key(deconstruct<Key>(e.key));
    lmdb::val lmdb_key(key), lmdb_val;
    bool found = txn.get(lmdb_key, lmdb_val);

    Metrics &m = metrics[threads.index() + 1]; // Metrics of current thread
    m.lookups++;

    if (found) {
        const Key k(e.key); // Copy for assert()
        e = reconstruct<Entry<Value> >(lmdb_val);
        assert(e.key == k);
    }

    return found;
}

template<typename Value>
inline
void
Database::update(const Entry<Value> &e, Transaction &txn)
{
    assert(e.key);

    Metrics &m = metrics[threads.index() + 1]; // Metrics of current thread
    const string key(deconstruct<Key>(e.key));
    lmdb::val lmdb_key(key);

    if (e.added || e.updated) {
        const string val(deconstruct<Entry<Value> >(e));
        lmdb::val lmdb_val(val);

        txn.put(lmdb_key, lmdb_val);
    } else if (e.removed) {
        txn.del(lmdb_key);
    }

    m.added += e.added;
    m.removed += e.removed;
    m.updated += e.updated;
}

inline
void
Database::update(const Traverse::Path &p)
{
    /*
     * Open the file, read the tags, add or update a DB entry (a key-value pair)
     */
    if (S_ISREG(p.info->st_mode)) {
        const Maintainer m(*this, p.name, p.info->st_mtime);
        threads.submit(m);
    }
}

} // verbatim
