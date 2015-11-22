#include "memory.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pwd.h>

#include <cstdlib>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <algorithm>

memory::~memory(void)
{
    if (start)
        delete[] start;
}

memory::memory(size_t size) noexcept(false) : size(size),
    start(NULL),
    ending(NULL)
{
    start = new char [size];
    ending = start + size;
}

void *memory::begin(void) noexcept
{
    return start;
}

void *const memory::end(void) const noexcept
{
    return ending;
}

size_t memory::cap(void) const noexcept
{
    return size;
}

char memory::operator[](size_t index) noexcept(false)
{
    if (cap() == 0)
        throw memory::null_allocation();
    if (index > (cap() - 1))
        throw memory::out_of_range();

    return *(start + index);
}

void memory::expand(size_t add) noexcept(false)
{
    if (cap() == 0)
        throw memory::null_allocation();
    if (index > (cap() - 1))
        throw memory::out_of_range();
 
    char *tmp = new char [size * 2];
    if (memcpy(tmp, start, size) == NULL)
        throw memory::expand_failed();
    std::swap(tmp, start);
    size = size * 2;
    ending = start + size;
}

