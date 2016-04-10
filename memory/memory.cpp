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
#include <cassert>

_memory::~_memory()
{
    if (start != NULL)
        delete[] start;

    start = current_position_write = current_position_read = ending = NULL;
    size = 0;
}

_memory::_memory()
: size(0)
, start(NULL)
, current_position_write(NULL)
, current_position_read(NULL)
, ending(NULL)
{ }

_memory::_memory(size_t size)
: size(size)
, start(NULL)
, current_position_write(NULL)
, current_position_read(NULL)
, ending(NULL)
{
    start = new char [size];
    current_position_write = start;
    current_position_read = start;
    ending = (start + size);
}

char *_memory::begin() noexcept
{
    return start;
}

char * const _memory::end() const noexcept
{
    return ending;
}

size_t _memory::cap() const noexcept
{
    return (size_t) (ending - size);
}

size_t _memory::get_read_offset() const noexcept
{
    return (size_t) (current_position_read - start);
}

size_t _memory::get_write_offset() const noexcept
{
    return (size_t) (current_position_write - start);
}

void _memory::write(const char *wr, size_t size) noexcept
{
    if (size == 0)
        return;

    assert(current_position_write != NULL);
    assert(wr != NULL);

    if ((get_write_offset() + size) <= cap())
        expand(cap()); // double the allocated size

    assert(memmove(current_position_write, wr, size) != NULL);
    current_position_write += size;
}

long _memory::read(char **buffer, size_t num_bytes) noexcept
{
    if (num_bytes == 0)
        return 0;

    assert(current_position_read != NULL);
    assert(buffer != NULL);
    assert(*buffer != NULL);

    assert(memmove(*buffer, current_position_read, num_bytes) != NULL);
    current_position_write += size;
}

void _memory::seek(ssize_t num_bytes, int mode) noexcept(false)
{
    switch(mode) {
    case seek_none:
    break;
    case seek_rd_current:
        current_position_read += num_bytes;
    break;
    case seek_wr_current:
        current_position_write += num_bytes;
    break;
    case seek_rd_begin:
        if (num_bytes < 0)
            throw memory::out_of_range();

        current_position_read = begin() + num_bytes;
    break;
    case seek_wr_begin:
        if (num_bytes < 0)
            throw memory::out_of_range();

        current_position_write = begin() + num_bytes;
    break;
    case seek_rd_end:
        if (num_bytes > 0)
            throw memory::out_of_range();

        current_position_read = end() + num_bytes;
    break;
    case seek_wr_end:
        if (num_bytes > 0)
            throw memory::out_of_range();

        current_position_write = end() + num_bytes;
    break;
    default:
    break;
    }
}

shared_memory memory::alloc(size_t size)
{
    return std::make_shared<_memory>(_memory(size));
}
