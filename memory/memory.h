#ifndef MEMORY_LMD9Q5J4

#define MEMORY_LMD9Q5J4

#ifdef _LINUX
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

void init_mem(void);

class memory {
    size_t size;
    void *start;
    void *ending;
#ifdef _LINUX
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
