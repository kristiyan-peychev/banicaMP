#ifndef MEMORY_LMD9Q5J4

#define MEMORY_LMD9Q5J4

#ifdef _LINUX
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

class memory {
    size_t size;
    void *start;
    void *ending;
#ifdef _LINUX
    int rnd_sht;
    key_t key;
    int shmid;
#endif
public:
    memory(void) = delete;
    memory(const memory &) = delete;
    memory &operator=(const memory &) = delete;
public:
    explicit memory(size_t) throw();
    ~memory(void);
public:
    // Change names?
    void *begin(void) noexcept;
    void * const end(void) const noexcept;
    size_t cap(void) const noexcept; // capacity
public:
    void expand(size_t) throw();
};

#endif /* end of include guard: MEMORY_LMD9Q5J4 */
