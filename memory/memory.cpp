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
    mem                             = new memory_core;
    mem->refs                       = 1;
    mem->size                       = size;
    mem->start                      = new char [size];
    mem->current_position_write     = mem->start;
    mem->current_position_read      = mem->start;
    mem->ending                     = mem->start + size;
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

size_t memory_ref::get_current_offset(void) const noexcept
{
    is_valid_throw();
    return mem->current_position_read - mem->start;
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
    if ((mem->current_position_write + write_size) >= end())
        throw memory::write_failed();

    if (memmove(mem->current_position_write, data, write_size) == NULL)
        throw memory::write_failed();
    mem->current_position_write = mem->current_position_write + write_size;
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

    return (mem->start + index);
}

char *memory_ref::read(size_t num_bytes) noexcept(false)
{
    is_valid_throw();
    if (cap() == 0)
        throw memory::null_allocation();
    if ((((size_t) (mem->current_position_read - mem->start)) + num_bytes) > (cap() - 1))
        throw memory::out_of_range();
    
    char *ret = mem->current_position_read + num_bytes;
    mem->current_position_read = mem->current_position_read + num_bytes;
    return ret;
}

long memory_ref::read(char **buffer, size_t num_bytes) noexcept(false)
{
    is_valid_throw();

    char *where;
    long ret;
    where = (char *) memcpy(*buffer, read(num_bytes), num_bytes);

    ret = (long) where - (long) mem->current_position_read;
    mem->current_position_read += ret;

    return ret;
}

void memory_ref::expand(size_t add) noexcept(false)
{
    is_valid_throw();
    if (cap() == 0 || add <= 0)
        throw memory::null_allocation();
 
    char *tmp = new char [mem->size + add];
    if (memcpy(tmp, mem->start, mem->size) == NULL)
        throw memory::expand_failed();

    size_t curpos_offset = mem->current_position_write - mem->start;
    std::swap(tmp, mem->start);
    mem->current_position_write = mem->start + curpos_offset;
    mem->size += add;
    mem->ending = mem->start + mem->size;
}

void memory_ref::seek(ssize_t num_bytes, int mode) noexcept(false)
{
    is_valid_throw();

    switch(mode) {
    case seek_none:
    break;
    case seek_rd_current:
        mem->current_position_read += num_bytes;
    break;
    case seek_wr_current:
        mem->current_position_write += num_bytes;
    break;
    case seek_rd_begin:
        if (num_bytes < 0)
            throw memory::out_of_range();

        mem->current_position_read = begin() + num_bytes;
    break;
    case seek_wr_begin:
        if (num_bytes < 0)
            throw memory::out_of_range();

        mem->current_position_write = begin() + num_bytes;
    break;
    case seek_rd_end:
        if (num_bytes > 0)
            throw memory::out_of_range();

        mem->current_position_read = end() + num_bytes;
    break;
    case seek_wr_end:
        if (num_bytes > 0)
            throw memory::out_of_range();

        mem->current_position_write = end() + num_bytes;
    break;
    default:
    break;
    }
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

