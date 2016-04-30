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
    return (size_t) (ending - start);
}

size_t _memory::get_read_offset() const noexcept
{
    return (size_t) (current_position_read - start);
}

size_t _memory::get_write_offset() const noexcept
{
    return (size_t) (current_position_write - start);
}

void _memory::write(const char *wr, size_t num_bytes) noexcept
{
    if (size == 0)
        return;

    assert(current_position_write != NULL);
    assert(wr != NULL);

    std::unique_lock<std::mutex> expand_lock(expand_mutex);
    expand_cond.wait(expand_lock,
                     [this]()
                     {
                        return !is_expanding;
                     });

    if (current_position_write + num_bytes >= ending)
        expand(cap()); // double the allocated size

    assert(memmove(current_position_write, wr, num_bytes) != NULL);
    current_position_write += num_bytes;
    m_cv.notify_one();
}

long _memory::read(char **buffer, size_t num_bytes) noexcept
{
    if (num_bytes == 0)
        return 0;
    else if (num_bytes > (cap() - get_read_offset()))
        num_bytes = cap() - get_read_offset();

    assert(current_position_read != NULL);
    assert(buffer != NULL);
    assert(*buffer != NULL);

    std::unique_lock<std::mutex> expand_lock(expand_mutex);
    expand_cond.wait(expand_lock,
                     [this]()
                     {
                        return !is_expanding;
                     });

    if (blocking_read && (get_read_offset() + num_bytes) >= get_write_offset()) {
        fprintf(stderr, "LOCKED, BITCH\n");
        std::unique_lock<std::mutex> lock(read_mutex);
        m_cv.wait(lock,
                  [this, num_bytes]()
                  {
                      return ((get_read_offset() + num_bytes) <
                               get_write_offset());
                  });
    }

    memmove(*buffer, current_position_read, num_bytes);
    current_position_read += num_bytes;

    return num_bytes;
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

void _memory::expand(size_t with_size) noexcept
{
    assert(with_size != 0);

    char *tmp;
    size_t write_offset = get_write_offset();
    size_t read_offset  = get_read_offset();

    tmp = new char [size + with_size];
    if (tmp == NULL)
        return;

    memmove(tmp, start, size);

    {
        is_expanding = true;
        std::lock_guard<std::mutex> lock(expand_mutex);
        std::swap(tmp, start);
        is_expanding = false;
        expand_cond.notify_all();
    }

    delete[] tmp;

    size += with_size;
    ending = begin() + size;
    current_position_write = begin() + write_offset;
    current_position_read  = begin() + read_offset;
}

void _memory::enable_blocking_read(bool block) noexcept
{
    blocking_read = block;
}

shared_memory memory::alloc(size_t size)
{
    return std::make_shared<_memory>(size);
}
