#ifndef MEMORY_LMD9Q5J4

#define MEMORY_LMD9Q5J4

#include <cstdlib>
#include <exception>
#include <stdexcept>

enum seek_control {
    seek_none = 0,
    seek_rd_current,
    seek_wr_current,
    seek_rd_begin,
    seek_wr_begin,
    seek_rd_end,
    seek_wr_end,
};

class memory_ref {
    struct memory_core {
        int      refs;
        size_t   size;
        char    *start;
        char    *current_position_write;
        char    *current_position_read;
        char    *ending;
    } *mem;
public:
    memory_ref(void)                            = delete;
public:
    ~memory_ref(void);
    memory_ref(memory_ref &);
    memory_ref &operator=(memory_ref &);
    memory_ref(size_t size) noexcept(false);
public:
    // Change names?
    char       *begin(void) noexcept;
    char *const end(void) const noexcept;
    size_t      cap(void) const noexcept; // capacity
    size_t      get_current_offset(void) const noexcept;
public:
    char        operator[](size_t) noexcept(false);
    void        write(const char *, size_t) noexcept(false);
    const char *read(size_t index, size_t num_bytes) noexcept(false);
    char       *read(size_t num_bytes) noexcept(false);
    long        read(char **buffer, size_t num_bytes) noexcept(false);
public:
    void        seek(ssize_t num_bytes, int mode) noexcept(false); //mode is from enum seek_control
public:
    void expand(size_t) noexcept(false);
public:
    bool is_valid() const noexcept;
    void is_valid_throw() const noexcept(false);
};

namespace memory {
    class exception : public std::exception { };
    class broken_ref : public memory::exception {
        public:
        const char *what(void) noexcept {
            return "Reference count of given memory was nonpositive";
        }
    };

    class out_of_range : public memory::exception {
        public:
        const char *what(void) noexcept {
            return "Index is out of range";
        }
    };

    class null_allocation : public memory::exception {
        public:
        const char *what(void) noexcept {
            return "The memory object holds an allocation of length zero";
        }
    };

    class expand_failed : public memory::exception {
        public:
        const char *what(void) noexcept {
            return "Memory expansion failed";
        }
    };
    class write_failed : public memory::exception {
        public:
        const char *what(void) noexcept {
            return "Memory write failed";
        }
    };
}

#endif /* end of include guard: MEMORY_LMD9Q5J4 */
