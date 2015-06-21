#define _WINDOWS
#include "memory.h"

// Before you start wondering why I did this -
// I did it for the sake of uniformity. It's going to be easier
// to use this as a container for the decoded songs than to 
// directly use two implementations which have _nothing_ in 
// common beside pointers.

memory::memory(size_t sz) throw() : size(sz)
{
    start = (void *) new char [size];
    ending = (void *) ((size_t) start + size);
}

memory::~memory(void)
{
    delete[] start;
}

void *memory::begin(void) noexcept
{
    return start;
}

void * const memory::end(void) const noexcept
{
    return ending;
}

size_t memory::cap(void) const noexcept
{
    return size;
}

void memory::expand(size_t add) throw()
{
    char *tmp = new char [size + add];

    memcpy(start, tmp, size);
    size += add;
}

