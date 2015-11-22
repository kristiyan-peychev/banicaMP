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

memory_ref::~memory_ref(void)
{
    --mem->refs;
    if ((mem->refs <= 0) && mem->start) {
        delete[] mem->start;
        delete[] mem;
    }
}

memory_ref::memory_ref(size_t size) noexcept(false)
{
    mem = new memory_core;
    mem->refs               = 1;
    mem->size               = size;
    mem->start              = new char [size];
    mem->current_position   = mem->start;
    mem->ending             = mem->start + size;
}

memory_ref::memory_ref(memory_ref &ref)
{
    if (ref.mem == NULL || ref.mem->refs <= 0)
        throw memory::broken_ref();
    mem = ref.mem;
    ++mem->refs;
}

memory_ref &memory_ref::operator=(memory_ref &ref)
{
    if (mem != NULL && ref.mem != NULL)
    {
        if (--mem->refs == 0)
            delete[] mem;
        mem = ref.mem;
        ++mem->refs;
    }
}

char *memory_ref::begin(void) noexcept
{
    is_valid_throw();
    return mem->start;
}

char *const memory_ref::end(void) const noexcept
{
    is_valid_throw();
    return mem->ending;
}

size_t memory_ref::cap(void) const noexcept
{
    is_valid_throw();
    return mem->size;
}

char memory_ref::operator[](size_t index) noexcept(false)
{
    is_valid_throw();
    if (cap() == 0)
        throw memory::null_allocation();
    if (index > (cap() - 1))
        throw memory::out_of_range();

    return *(mem->start + index);
}

void memory_ref::write(const char *data, size_t write_size) noexcept(false)
{
    is_valid_throw();
    if (cap() == 0)
        throw memory::null_allocation();
    if ((mem->current_position + write_size) >= end())
        throw memory::out_of_range();

    if ((mem->current_position = (char *)
            memmove(mem->current_position, data, write_size)) == NULL)
        throw memory::write_failed();
}

const char *memory_ref::read(size_t index, size_t num_bytes) noexcept(false)
{
    is_valid_throw();
    if (cap() == 0)
        throw memory::null_allocation();
    if ((index + num_bytes) > (cap() - 1))
        throw memory::out_of_range();
    if (index > (cap() - 1))
        throw memory::out_of_range();
    // operator[] will check for null-allocation and if index is out of range
    return (mem->start + index);
}

void memory_ref::expand(size_t add) noexcept(false)
{
    is_valid_throw();
    if (cap() == 0 || add == 0)
        throw memory::null_allocation();
    if (add > (cap() - 1))
        throw memory::out_of_range();
 
    char *tmp = new char [mem->size + add];
    if (memcpy(tmp, mem->start, mem->size) == NULL)
        throw memory::expand_failed();

    size_t curpos_offset = mem->current_position - mem->start;
    std::swap(tmp, mem->start);
    mem->current_position = mem->start + curpos_offset;
    mem->size += add;
    mem->ending = mem->start + mem->size;
}

bool memory_ref::is_valid() const noexcept
{
    return (mem != NULL);
}

void memory_ref::is_valid_throw() const noexcept(false)
{
    if (!is_valid())
        throw memory::broken_ref();
}

