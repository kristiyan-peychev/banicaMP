#ifndef FILTER_K987NIL0

#define FILTER_K987NIL0

#include "../filter.hpp"

#include <sys/types.h>

template <class C = float, unsigned size = 10>
class equalizer : public filter<C, size> {
    C list[size];
public:
    filter(void) { }
public:
    inline const C *get_list(void) const { return list; }
    inline const unsigned get_size(void) const { return size; }
    inline void set_list(const C *ar, size_t sz) {
        while (sz)
            list[--sz] = ar[sz];
    }
};

#endif /* end of include guard: FILTER_K987NIL0 */
