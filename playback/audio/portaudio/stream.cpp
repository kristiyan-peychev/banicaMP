#include "stream.h"
#include "audio.h"
#include "../../../memory/memory.h"
#include <cstring>
#include <stdio.h>

#define spinlock(x) do { } while(x.test_and_set(std::memory_order_acquire))

stream::buffer::~buffer()
{
    if (NULL != data)
        delete[] data;
}

stream::buffer::buffer(size_t alloc)
: data(new char [alloc])
, size(alloc)
{ }

stream::buffer &stream::buffer::operator=(const stream::buffer &other)
{
    if (&other == this)
        return *this;

    if (NULL != data)
        delete[] data;

    size = other.size;
    data = new char [other.size];
    memcpy(data, other.data, size);

    return *this;
}

stream::buffer_manager::~buffer_manager()
{
    die_flag = true; // kill the thread
    lock.clear(std::memory_order_release);
    prefill_thread.join();
}

stream::buffer_manager::buffer_manager(stream * st, size_t num_buffers)
: buffers(num_buffers)
, read(st)
, lock(ATOMIC_FLAG_INIT)
, next_fill_index(0)
, die_flag(false)
{
    buffer tmp(FRAMES_PER_BUFFER);
    for (int i = 0; i < 2; ++i)
    {
        buffers[i] = tmp;
        read->fill_buffer(buffers[i], FRAMES_PER_BUFFER);
    }

    lock.test_and_set(); // lock the thread on its spinlock
    prefill_thread = std::thread(&stream::buffer_manager::prebuffer, this);
}

void stream::buffer_manager::fill_buffer(buffer &buf)
{
    buf = buffers[next_fill_index];
    lock.clear(std::memory_order_release);
}

void stream::buffer_manager::fill_buffer(char **buf, size_t size)
{
    if (buf == NULL || *buf == NULL || size == 0)
        return;

    memcpy(*buf, buffers[next_fill_index].data, size);
    fprintf(stderr, "Releasing lock\n");
    lock.clear(std::memory_order_release);
    fprintf(stderr, "Released lock\n");
}

// fill whenever one of the buffers is read
void stream::buffer_manager::prebuffer()
{
    size_t last_read = 0;
    do {
        fprintf(stderr, "l0ck\n");
        spinlock(lock);

        if (die_flag)
            return;

        read->fill_buffer(buffers[next_fill_index], buffers.size());
        fprintf(stderr, "Filled @#%d\n", next_fill_index);
        last_read = buffers[next_fill_index].size;
        next_fill_index = (next_fill_index + 1) % buffers.size();
    } while(last_read > 0);
}

stream::~stream()
{ }

stream::stream()
: manager(this)
, state(stream_state_none)
, source_file(NULL)
, source_memory(0)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(FILE *source, int prebuffer_num)
: manager(this, prebuffer_num)
, state(stream_state_none)
, source_file(source)
, source_memory(0)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(memory_ref source, int prebuffer_num)
: manager(this, prebuffer_num)
, state(stream_state_memory)
, source_file(NULL)
, source_memory(source)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(std::fstream &&source, int prebuffer_num)
: manager(this, prebuffer_num)
, state(stream_state_fstream)
, source_file(NULL)
, source_memory(0)
, source_fstream(std::move(source))
, next_read_size(0)
{ }

stream_state stream::get_state() const
{
    return state;
}

bool stream::get_flag(int mask) const
{
    return state & mask;
}

long stream::size()
{
    long ret;

    std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
    break;
    case stream_state_file:
    {
        long current_offset = ftell(source_file);
        fseek(source_file, 0, SEEK_END);
        ret = ftell(source_file);
        ret -= current_offset;
        fseek(source_file, current_offset, SEEK_SET);
    }
    break;
    case stream_state_memory:
        return source_memory.get_current_offset();
    break;
    case stream_state_fstream:
    {
        long current_offset = source_fstream.tellg();
        source_fstream.seekg(0, std::ios::end);
        ret = source_fstream.tellg();
        ret -= current_offset;
        source_fstream.seekg(current_offset, std::ios::beg);
    }
    break;
    }
    return ret;
}

long stream::get_size()
{
    return size();
}

long stream::get_remaining()
{
    long ret;
 
    //std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
    break;
    case stream_state_file:
    {
        long current_offset = ftell(source_file);
        fseek(source_file, 0, SEEK_END);
        ret = ftell(source_file);
        ret -= current_offset;
        fseek(source_file, current_offset, SEEK_SET);
    }
    break;
    case stream_state_memory:
        return source_memory.get_current_offset();
    break;
    case stream_state_fstream:
    {
        long current_offset = source_fstream.tellg();
        source_fstream.seekg(0, std::ios::end);
        ret = source_fstream.tellg();
        ret -= current_offset;
        source_fstream.seekg(current_offset, std::ios::beg);
    }
    break;
    }
    return ret;
}

long stream::fill_buffer(char **buf, unsigned long bytes)
{
    if (buf == NULL || *buf == NULL || bytes == 0)
        return -1;

    long copy_size = bytes;
    unsigned long remain = get_remaining();
    if (remain < bytes)
        copy_size = remain;

    //std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
    break;
    case stream_state_file:
    {
        buffer buff;
        buff.data = *buf;
        buff.size = fread(buff.data, 1, copy_size, source_file);
        return buff.size;
    }
    break;
    case stream_state_memory:
    {
        buffer buff;
        buff.data = *buf;
        manager.fill_buffer(buff);
        return buff.size;
    }
    break;
    case stream_state_fstream:
        source_fstream.read(*buf, copy_size);
        return source_fstream.gcount();
    break;
    }
    
    return -1; // should never be reached
}

long stream::seek(long bytes)
{
    long seek_len = bytes;
    long remain = get_remaining();
    if (remain < bytes)
        seek_len = remain;

    std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
    break;
    case stream_state_file:
        if (fseek(source_file, seek_len, SEEK_CUR) != 0)
            return -1;
    break;
    case stream_state_memory:
        try {
            source_memory.seek(seek_len, seek_rd_current);
        } catch(memory::exception &e) {
            return -1;
        }
    break;
    case stream_state_fstream:
        source_fstream.seekg(seek_len, std::ios::cur);
    break;
    }

    return seek_len;
}

long stream::rewind()
{
    std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
    break;
    case stream_state_file:
        if (fseek(source_file, 0, SEEK_SET) != 0)
            return -1;
    break;
    case stream_state_memory:
        try {
            source_memory.seek(0, seek_rd_begin);
        } catch(memory::exception &e) {
            return -1;
        }
    break;
    case stream_state_fstream:
        source_fstream.seekg(0, std::ios::beg);
    break;
    }

    return 0;
}

void stream::fill_buffer(buffer &buf, size_t bytes)
{
    long copy_size = bytes;
    unsigned long remain = get_remaining();
    if (remain < bytes)
        copy_size = remain;

    //std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
        return;
    break;
    case stream_state_file:
        buf.size = fread(buf.data, 1, copy_size, source_file);
    break;
    case stream_state_memory:
        buf.size = source_memory.read(&buf.data, copy_size);
    break;
    case stream_state_fstream:
        source_fstream.read(buf.data, copy_size);
        buf.size = source_fstream.gcount();
    break;
    }
}

stream &operator<<(stream &st, long next_read_bytes)
{
    st.next_read_size = next_read_bytes;
    return st;
}

stream &operator>>(stream &st, char **buffer_to_fill)
{
    if (st.next_read_size <= 0)
        return st;

    st.fill_buffer(buffer_to_fill, st.next_read_size);
    st.next_read_size = 0;
    return st;
}

