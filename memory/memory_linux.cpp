#define _LINUX
#include "memory.h"
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>

#include <cstdlib>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <algorithm>

#define NUMBER 128

class not_implemented {
    const char *what(void) const noexcept {
        return "Not implemented under this OS.";
    }
};

static bool inited = false;
const char *homedir;

void init_mem(void)
{
    if (!inited) {
        if ((homedir = getenv("HOME")) == NULL)
            homedir = getpwuid(getuid())->pw_dir;

        inited = true;
    }
}

static const char next = '&'; // assuming only one song will play at one time

memory::memory(size_t size, const char *segname) noexcept(false) : size(size)
{
    //if (size <= 128) // you're retarded for asking for that little memory
        //throw std::bad_alloc(); // throw something else?
    int fd;
    init_mem();

    segpath = (char *) malloc((strlen(homedir) + strlen(segname) + 16) * sizeof(*segpath));
    if (segpath == NULL)
        throw std::bad_alloc("failed to allocate memory");
    sprintf(segpath, "%s/.banicamp", homedir);

    fd = mkdir(segpath, 0644);
    if (fd != 0 && fd != EEXIST) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    sprintf(segpath, "/%s", segname);
    fd = open(segpath, O_CREAT | O_EXCL | 0644);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    key = ftok(segpath, next); // FIXME
    if (key == -1) {
        perror("ftok");
        throw std::bad_alloc("could not get a segment from file");
    }

    shmid = shmget(key, this->size, 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        throw std::bad_alloc("count not get a segment from key");
    }

    start = (char *) shmat(shmid, (void *) 0, 0);
    ending = (void *) ((size_t) start + size);
    if (start == (void *) (-1)) {
        perror("shmat");
        throw std::bad_alloc("failed to get a pointer in the shared segment");
    }
}

memory::~memory(void)
{
    shmctl(shmid, IPC_RMID, NULL);
    remove(segpath); // let's hope this doesn't crash everything
    free(segpath);
}

void *memory::begin(void) noexcept
{
    return start;
}

inline void * const memory::end(void) const noexcept
{
    return ending;
}

size_t memory::cap(void) const noexcept
{
    return size;
}

void memory::expand(size_t add) noexcept(false)
{
    throw not_implemented();
    return;
}

