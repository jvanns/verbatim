/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_DELEGATE_HPP
#define VERBATIM_UTILITY_DELEGATE_HPP

namespace verbatim {
namespace utility {

template <typename Class, typename In, typename Out>
class Delegate
{
    private:
        /* Dependent typedefs */
        typedef void (Class::*Fn)(const In&, Out&);

        /* Member functions */
        Delegate();
        Delegate(const Delegate &d);

        /* Member attributes */
        Class &target;
        Fn callback;
    public:
        /* Member functions */
        explicit Delegate(Class &c, Fn f) : target(c), callback(f) {}
        void operator() (const In &i, Out &o) { (target.*callback)(i, o); }
};

/*
 * Specialisation for no result store
 */
template <typename Class, typename In>
class Delegate<Class, In, void>
{
    private:
        /* Dependent typedefs */
        typedef void (Class::*Fn)(const In&);

        /* Member functions */
        Delegate();
        Delegate(const Delegate &d);

        /* Member attributes */
        Class &target;
        Fn callback;
    public:
        /* Member functions */
        explicit Delegate(Class &c, Fn f) : target(c), callback(f) {}
        void operator() (const In &i) { (target.*callback)(i); }
};

} // utility
} // verbatim

#endif // VERBATIM_UTILITY_DELEGATE_HPP

