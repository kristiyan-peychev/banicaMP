#ifndef STREAM_74S8YR1

#define STREAM_74S8YR1

#include <mutex>
#include <fstream>

#include "../../../memory/memory.h"

typedef enum {
    stream_state_none       = 0,
    stream_state_not_init   = 1,
    stream_state_file       = 2,
    stream_state_memory     = 4,
    stream_state_fstream    = 8,
} stream_state;

class stream {
    stream_state state;
    FILE        *source_file;
    memory_ref   source_memory;
    std::fstream source_fstream;
    long         next_read_size;

    std::mutex   mutex;
public:
   ~stream();

    stream();
    stream(FILE *source);
    stream(memory_ref source);
    stream(std::fstream &&source);
public:
    stream_state get_state() const;
    bool         get_flag(int mask) const;
public:
    long size();
    long get_size(); // same as size()
    long get_remaining();
    long fill_buffer(char **buffer, unsigned long bytes);
    long seek(long bytes);
    long rewind();
public:
    friend stream &operator<<(stream &st, long next_read_bytes);
    friend stream &operator>>(stream &st, char **buffer_to_fill);
};

#endif /* end of include guard: STREAM_74S8YR1 */
