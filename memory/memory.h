#ifndef MEMORY_LMD9Q5J4

#define MEMORY_LMD9Q5J4

#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <memory>

class _memory {
    size_t   size;
    char    *start;
    char    *current_position_write;
    char    *current_position_read;
    char    *ending;
public:
   ~_memory();
    _memory();
    _memory(size_t size);
public:
    char       *begin(void) noexcept;
    char *const end(void) const noexcept;
    size_t      cap(void) const noexcept; // capacity
    size_t      get_read_offset(void) const noexcept;
    size_t      get_write_offset(void) const noexcept;
public:
    void        write(const char *, size_t) noexcept;
    long        read(char **buffer, size_t num_bytes) noexcept;
public:
    void        seek(ssize_t num_bytes, int mode) noexcept(false); //mode is from enum seek_control
public:
    void expand(size_t with_size) noexcept;
};

typedef std::shared_ptr<_memory> shared_memory;

namespace memory {
    shared_memory alloc(size_t size);
}

enum seek_control {
    seek_none = 0,
    seek_rd_current,
    seek_wr_current,
    seek_rd_begin,
    seek_wr_begin,
    seek_rd_end,
    seek_wr_end,
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
