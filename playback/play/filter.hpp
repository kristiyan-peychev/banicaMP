#ifndef FILTER_K987NIL0

#define FILTER_K987NIL0

#include <sys/types.h>

template <class C = float, unsigned size = 10>
class filter {
    filter(void);
public:
    const C *get_list(void) const = 0;
    inline const unsigned get_size(void) const { return size; }
    void set_list(const C *ar, size_t sz) = 0;
};

#endif /* end of include guard: FILTER_K987NIL0 */
