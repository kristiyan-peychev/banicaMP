#ifndef MEMORY_LMD9Q5J4

#define MEMORY_LMD9Q5J4

#include <cstdlib>
#include <exception>

class memory {
    size_t   size;
    void    *start;
    void    *ending;
public:
    memory(void)                        = delete;
    memory(const memory &)              = delete;
    memory &operator=(const memory &)   = delete;
public:
    ~memory(void);
    memory(size_t) noexcept(false);
public:
    // Change names?
    void       *begin(void) noexcept;
    void       *const end(void) const noexcept;
    size_t      cap(void) const noexcept; // capacity
public:
    char        operator[](size_t) noexcept(false);
public:
    void expand(size_t) noexcept(false);
};

namespace memory {
    class out_of_range : public std::out_of_range {
    };

    class null_allocation : public std::exception {
        const char *what(void) noexcept {
            return "The memory object holds an allocation of length zero";
        }
    };

    class expand_failed : public std::exception {
        const char *what(void) noexcept {
            return "Memory expansion failed";
        }
    };
}

#endif /* end of include guard: MEMORY_LMD9Q5J4 */
