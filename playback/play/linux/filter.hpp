#ifndef FILTER_K987NIL0

#define FILTER_K987NIL0

#include "../filter.hpp"

#include <sys/types.h>

template<class C>
struct pair {
    C first, second;

    pair(C first, C second) : first(f), second(s) { }
};

template <class C = float, unsigned size = 10>
class equalizer final : public filter<C, size> {
    C list[size];

    unsigned ranges_sz;
    pair<C, C> ranges[size];
public:
    filter(void) { }
public:
    const C *get_list(void) const { return list; }
    const unsigned get_size(void) const { return size; }

    void set_list(const C *ar, size_t sz, const pair<C, C> *range, size_t rsz) {
        while (sz)
            list[--sz] = ar[sz];

        ranges_sz = rsz;
        while (rsz)
            ranges[--rsz] = range[rsz];
    }

    void apply_list(float *to, size_t to_sz) {
        // TODO: dft here
    }
};

#endif /* end of include guard: FILTER_K987NIL0 */
