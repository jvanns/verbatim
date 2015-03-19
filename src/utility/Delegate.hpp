/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

#ifndef VERBATIM_UTILITY_DELEGATE_HPP
#define VERBATIM_UTILITY_DELEGATE_HPP

// libstdc++
#include <list>
#include <utility>

namespace verbatim {
namespace utility {

struct Observer {
    virtual ~Observer() {};
};

template <typename In, typename Out>
class Delegate
{
    private:
        /* Dependent typedefs */
        typedef void (Observer::*Method)(const In&, Out&);
        typedef std::pair<Observer*, Method> Item;
        typedef std::list<Item> Observers;

        /* Member attributes */
        Observers observers;
    public:
        /* Member functions */
        template<typename ObserverImpl>
        inline void connect(ObserverImpl *o,
                            void (ObserverImpl::*m)(const In&, Out&))
        {
            observers.push_back(std::make_pair(o, static_cast<Method>(m)));
        }

        inline void dispatch(const In &i, Out &o)
        {
            typename Observers::iterator b(observers.begin()),
                                         e(observers.end());

            while (b != e) {
                Observer &object = *(b->first);
                Method method = b->second;
                (object.*method)(i, o);
                ++b;
            }
        }
};

/*
 * Specialisation for no result store
 */
template <typename In>
class Delegate<In, void>
{
    private:
        /* Dependent typedefs */
        typedef void (Observer::*Method)(const In&);
        typedef std::pair<Observer*, Method> Item;
        typedef std::list<Item> Observers;

        /* Member attributes */
        Observers observers;
    public:
        /* Member functions */
        template<typename ObserverImpl>
        inline void connect(ObserverImpl *o,
                            void (ObserverImpl::*m)(const In&))
        {
            observers.push_back(std::make_pair(o, static_cast<Method>(m)));
        }

        inline void dispatch(const In &i)
        {
            typename Observers::iterator b(observers.begin()),
                                         e(observers.end());

            while (b != e) {
                Observer &object = *(b->first);
                Method method = b->second;
                (object.*method)(i);
                ++b;
            }
        }
};

} // utility
} // verbatim

#endif // VERBATIM_UTILITY_DELEGATE_HPP

