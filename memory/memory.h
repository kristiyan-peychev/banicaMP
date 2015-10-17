#ifndef MEMORY_LMD9Q5J4

#define MEMORY_LMD9Q5J4

#if defined(__linux__)
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include <cstdlib>

void init_mem(void);

class memory {
    size_t size;
    void *start;
    void *ending;
#if defined(__linux__)
    char *segpath;
    int rnd_sht;
    key_t key;
    int shmid;
#endif
public:
    memory(void) = delete;
    memory(const memory &) = delete;
    memory &operator=(const memory &) = delete;
public:
    explicit memory(size_t, const char *) noexcept(false);
    ~memory(void);
public:
    // Change names?
    void *begin(void) noexcept;
    void * const end(void) const noexcept;
    size_t cap(void) const noexcept; // capacity
public:
    void expand(size_t) noexcept(false);
};

#endif /* end of include guard: MEMORY_LMD9Q5J4 */
