#include "stream.h"
#include "../../../memory/memory.h"

stream::~stream()
{ }

stream::stream()
: state(stream_state_not_init)
, source_file(NULL)
, source_memory(0)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(FILE *source)
: state(stream_state_file)
, source_file(source)
, source_memory(0)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(memory_ref source)
: state(stream_state_memory)
, source_file(NULL)
, source_memory(source)
, source_fstream()
, next_read_size(0)
{ }

stream::stream(std::fstream &&source)
: state(stream_state_fstream)
, source_file(NULL)
, source_memory(0)
, source_fstream(std::move(source))
, next_read_size(0)
{ }

stream_state stream::get_state() const
{ return state;
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
        return -1;
    break;
    case stream_state_not_init:
        return -1;
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
 
    std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
        return -1;
    break;
    case stream_state_not_init:
        return -1;
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

long stream::fill_buffer(char **buffer, unsigned long bytes)
{
    if (buffer == NULL || *buffer == NULL || bytes == 0)
        return -1;

    long copy_size = bytes;
    unsigned long remain = get_remaining();
    if (remain < bytes)
        copy_size = remain;

    std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
        return -1;
    break;
    case stream_state_not_init:
        return -1;
    break;
    case stream_state_file:
        return fread(*buffer, 1, copy_size, source_file);
    break;
    case stream_state_memory:
        return source_memory.read(buffer, copy_size);
    break;
    case stream_state_fstream:
        source_fstream.read(*buffer, copy_size);
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
        return -1;
    break;
    case stream_state_not_init:
        return -1;
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

long stream::seek(long bytes)
{
    long seek_len = bytes;
    long remain = get_remaining();
    if (remain < bytes)
        seek_len = remain;

    std::lock_guard<std::mutex> guard(mutex);
    switch (state) {
    case stream_state_none:
        return -1;
    break;
    case stream_state_not_init:
        return -1;
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

    return seek_len;
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

