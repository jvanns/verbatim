/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// libstdc++
#include <limits>
#include <vector>
#include <iostream>
#include <algorithm>

// libc
#include <stdint.h>

// stl
using std::cout;
using std::endl;
using std::sort;
using std::vector;
using std::ostream;
using std::lower_bound;
using std::numeric_limits;

typedef uint32_t Integer;
static const uint8_t digits = numeric_limits<Integer>::digits;

struct Suffix
{
    /*
     * n:      The number or integer suffix
     * index:  The index or offset of this suffix relative to source integer
     * lcp:    The longest common prefix (shared with an immediate predecessor)
     */

    Integer n;
    size_t index, lcp;

    Suffix(Integer i = 0,
           size_t j = 0,
           size_t k = 0) : n(i), index(j), lcp(k) {}

    bool operator< (Integer other) const { return n < other; }
    bool operator< (const Suffix &other) const { return n < other.n; }
};

inline
void
print_as_bin(ostream &s, Integer n)
{
    for (uint8_t i = 1 ; i <= digits ; ++i) {
        const uint16_t bit = n & (1 << (digits - i)) ? 1 : 0;
        s << bit;
    }
}

inline
size_t
lzc(Integer n) // Leading Zero Count
{
    static const Integer max = numeric_limits<Integer>::max();
    static const uint8_t shift = digits - 1;
    size_t leading_zeros = 0;

    /*
     * I'm expecting the compiler to unroll this loop when built with -O[1-3]
     */
    while (n && static_cast<Integer>(~(n >> shift)) == max) {
        ++leading_zeros;
        n <<= 1;
    }

    return leading_zeros;
}

ostream& operator<< (ostream &os, const Suffix &s)
{
    os << s.index << '\t';
    print_as_bin(os, s.n);
    os << '\t' << s.lcp;
    return os;
}

void
compute_suffix_array(const Integer n)
{
    vector<Suffix> v;
    v.reserve(digits);

    for (uint8_t i = 1 ; i <= digits ; ++i) {
        const Integer trunc = n << (digits - i);

        if (!trunc)
            continue; // Ignore 0

        vector<Suffix>::iterator b(v.begin()),
                                 e(v.end()),
                                 j(lower_bound(b, e, trunc));

        if (j == e || trunc < j->n) {
            const size_t x = j - b, p = x ? lzc(trunc ^ v[x - 1].n) : 0;

            j = (v.insert(j, Suffix(trunc, digits - i, p))) + 1;
            j->lcp = lzc(trunc ^ j->n);
        }

        if ((trunc >> (digits - i)) == n)
            break; // Bail early, don't cycle through all bits
    }
    
    /*
     * Print our suffix array for n
     */
    cout << Suffix(n, 0, 0) << endl; // Source/input integer
    for (size_t i = 0 ; i < v.size() ; ++i)
        cout << v[i] << endl;
}

int main(int argc, char *argv[])
{
    while(++argv && *argv) {
        const Integer n = static_cast<Integer>(strtoul(*argv, NULL, 10));
        compute_suffix_array(n);
    }

    return 0;
}

