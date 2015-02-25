/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// verbatim
#include "Traverse.hpp"

// libstdc++
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

namespace {

/*
 * Traversal callback no. 1
 */
class RegisterPath : public verbatim::Traverse::Callback
{
    public:
        RegisterPath() : tally(0) {}
        ~RegisterPath() { cout << "Received " << tally << " paths\n"; }

        void operator() (const verbatim::Traverse::Path &p)
        {
            cout << p.name << endl;
            ++tally;
        }
    private:
        size_t tally;
};

/*
 * Traversal callback no. 2
 */
class AggregateSize : public verbatim::Traverse::Callback
{
    public:
        AggregateSize() : size(0) {}
        ~AggregateSize()
        {
            cout << "Aggregate file size: "
                 << size / static_cast<float>(1 << 20)
                 << " MB\n";
        }

        void operator() (const verbatim::Traverse::Path &p)
        {
            if (S_ISREG(p.info->st_mode))
                size += p.info->st_size;
        }
    private:
        size_t size;
};

} // anonymous

int main(int argc, char *argv[])
{
    using verbatim::Traverse;

    if (!argv[1]) {
        cerr << "Provide a source 'root' path" << endl;
        return 1;
    }

    Traverse t;
    RegisterPath callback1;
    AggregateSize callback2;

    t.register_callback(&callback1);
    t.register_callback(&callback2);
    t.scan(argv[1]);

    return 0;
}

