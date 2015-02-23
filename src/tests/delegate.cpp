/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// verbatim
#include "utility/Delegate.hpp"

// libstdc++
#include <iostream>

// libc
#include <assert.h>

namespace {

/*
 * Example callbacks
 */
class ExampleA
{
    public:
        void set(const int &i)
        {
            n = i;
        }

        inline const int& get() const { return n; }
    private:
        int n;
};

class ExampleB
{
    public:
        void normalise(const int &i, double &out)
        {
            const double x = 1.0 / static_cast<double>(i);
            out = x * x;
        }
};

struct Foo {
    short a;
    Foo(short x) : a(x) {}
};

struct Bar {
    short b;
};

class ExampleC
{
    public:
        void copy(const Foo &f, Bar &b)
        {
            b.b = f.a;
        }
};

} // anonymous

int main(int argc, char *argv[])
{
    using std::cout;
    using std::endl;
    using verbatim::utility::Delegate;

    {
        ExampleA e;
        Delegate<ExampleA, int, void> cbk(e, &ExampleA::set);

        cbk(8);
        assert(e.get() == 8);
    }

    {
        ExampleB e;
        int i = 42;
        double result;
        Delegate<ExampleB, int, double> cbk(e, &ExampleB::normalise);

        cbk(i, result);
        assert(result >= 0.0f && result <= 1.0f);
    }

    {
        const Foo f (19);
        Bar b;
        ExampleC e;
        Delegate<ExampleC, Foo, Bar> cbk(e, &ExampleC::copy);

        cbk(f, b);
        assert(b.b == f.a);
    }

    return 0;
}

