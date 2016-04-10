#ifndef STREAM_74S8YR1

#define STREAM_74S8YR1

#include <fstream>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

#include "../../../memory/memory.h"

typedef enum {
    stream_state_none       = 0,
    stream_state_file       = 1,
    stream_state_memory     = 2,
    stream_state_fstream    = 4,
} stream_state;

class stream {
public:
    struct buffer {
        char *data;
        size_t size;
        bool del;
    public:
       ~buffer();
        buffer();
        buffer(char **buff, size_t sz, bool delet = false);
        buffer &operator=(const buffer &other);
    };

    class buffer_manager {
        std::vector<buffer> buffers;
        stream             *read;
        std::atomic_flag    lock;
        std::thread         prefill_thread;
        size_t              next_fill_index;
        bool                die_flag;
    public:
       ~buffer_manager();
        buffer_manager(stream *st, size_t num_buffers = 4);
    public:
        void fill_buffer(buffer &buf);
        void fill_buffer(char **buf, size_t size);
    protected:
        void prebuffer();
    } manager;

    stream_state state;
    FILE        *source_file;
    memory_ref   source_memory;
    std::fstream source_fstream;
    long         next_read_size;

    std::mutex   mutex;
public:
   ~stream();

    stream();
    stream(FILE *source, int prebuffer_num = 4);
    stream(memory_ref source, int prebuffer_num = 2);
    stream(std::fstream &&source, int prebuffer_num = 2);
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
//protected:
    void fill_buffer(buffer &buf, size_t bytes);
public:
    friend stream &operator<<(stream &st, long next_read_bytes);
    friend stream &operator>>(stream &st, char **buffer_to_fill);
};

#endif /* end of include guard: STREAM_74S8YR1 */
