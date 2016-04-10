#include "stream.h"
#include "audio.h"
#include "../../../memory/memory.h"
#include <cstring>
#include <stdio.h>

#define spinlock(x) do { } while(x.test_and_set(std::memory_order_acquire))

stream::buffer::~buffer()
{
    if (NULL != data && del)
        delete[] data;
}

stream::buffer::buffer()
: data(NULL)
, size(0)
{ }

stream::buffer::buffer(char **buff, size_t sz, bool delet)
: data(*buff)
, size(sz)
, del(delet)
{ }

stream::buffer &stream::buffer::operator=(const stream::buffer &other)
{
    if (this == &other)
        return *this;

    if (del)
        delete data;

    data = other.data;
    size = other.size;
    del = other.del;
    return *this;
}

stream::buffer_manager::~buffer_manager()
{
    die_flag = true; // kill the thread
    lock.clear(std::memory_order_release);
    prefill_thread.join();
}

stream::buffer_manager::buffer_manager(stream *st, size_t num_buffers)
: buffers(num_buffers)
, read(st)
, lock(ATOMIC_FLAG_INIT)
, next_fill_index(0)
, die_flag(false)
{
    for (int i = 0; i < buffers.size(); ++i)
    {
        char *val = new char [FRAMES_PER_BUFFER];
        buffers[i] = buffer(&val, FRAMES_PER_BUFFER);
        read->fill_buffer(buffers[i], FRAMES_PER_BUFFER);
        buffers[i].del = true;
    }

    lock.test_and_set(std::memory_order_acquire); // lock the thread on its spinlock
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
    lock.clear(std::memory_order_release);
}

// fill whenever one of the buffers is read
void stream::buffer_manager::prebuffer()
{
    size_t last_read = 0;
    do {
        spinlock(lock);

        if (die_flag)
            return;

        read->fill_buffer(buffers[next_fill_index], buffers[next_fill_index].size);
        last_read = buffers[next_fill_index].size;
        next_fill_index = (next_fill_index + 1) % buffers.size();
    } while(last_read > 0 && !die_flag);
}

stream::~stream()
{ }

stream::stream()
: manager(this)
, state(stream_state_none)
, source_file(NULL)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(FILE *source, int prebuffer_num)
: manager(this, prebuffer_num)
, state(stream_state_file)
, source_file(source)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(shared_memory source, int prebuffer_num)
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
        return source_memory->get_read_offset();
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
        return source_memory->get_read_offset();
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

    manager.fill_buffer(buf, bytes);
    return bytes;
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
            source_memory->seek(seek_len, seek_rd_current);
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
            source_memory->seek(0, seek_rd_begin);
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
        buf.size = source_memory->read(&buf.data, copy_size);
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

