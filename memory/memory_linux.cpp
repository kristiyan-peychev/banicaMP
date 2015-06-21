#define _LINUX
#include "memory.h"
#include <stdlib.h>
#include <exception>
#include <new>
#include <algorithm>

#define PATH_TO_IDENTIFIER ""

class not_implemented {
    const char *what(void) const noexcept {
        return "Not implemented under this OS."
    }
};

static const char next = '&'; // assuming only one song will play at one time

memory::memory(size_t size) throw() : size(size)
{
    //if (size <= 128) // you're retarded for asking for that little memory
        //throw std::bad_alloc(); // throw something else?

    key = ftok(PATH_TO_IDENTIFIER, next);
    if (key == -1)
        throw std::bad_alloc();

    shmid = shmget(key, this->size, 0644 | IPC_CREAT);
    if (shmid == -1)
        throw std::bad_alloc();

    start = (char *) shmat(shmid, (void *) 0, 0);
    ending = start + size;
    if (start == (void *) (-1))
        throw std::bad_alloc(); // same?
}

memory::~memory(void)
{
    shmctl(shmid, IPC_RMID, NULL);
}

inline void *memory::get(void) noexcept
{
    return start;
}

inline void * const memory::end(void) const noexcept
{
    return ending;
}

inline size_t memory::cap(void) const noexcept
{
    return size;
}

void memory::expand(size_t add) throw()
{
    throw not_implemented();
    return;
}

