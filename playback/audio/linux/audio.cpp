#include "audio.h"
//#include "aplay.h"

#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <exception>

#define PARENT_PIPE_NAME "banica_player_chld_ctl_fifo"
#define APLAY_ARG_COUNT 7

static const char *aplay_args[] = {
    "/aplay", // executable name
    "-f", "cd", // 16-bit signed integer
    "-t", "wav", // WAVE file
    "-q", // quiet mode
    "-" // read from stdin
};

alsa_wav_player::~alsa_wav_player(void)
{
    stop();
    close(child_pipe);
}

alsa_wav_player::alsa_wav_player(FILE *filep) :
    is_paused(true), childpid(0), child_pipe(0), input_mem(0),
    flg(FLG_FILE)
{
    input_fd = fileno(filep);
}

alsa_wav_player::alsa_wav_player(memory_ref &input) :
    is_paused(true), childpid(0), input_fd(-1), input_mem(input),
    flg(FLG_MEM)
{ }

alsa_wav_player::alsa_wav_player(int fd) :
    is_paused(true), childpid(0), input_fd(fd), child_pipe(0), input_mem(0),
    flg(FLG_FILE)
{ }

void alsa_wav_player::begin(void)
{
    if (flg & FLG_MEM)
        begin_mem();
    else if (flg & FLG_FILE)
        begin_file();
}

void alsa_wav_player::play(void)
{
    if (flg & FLG_MEM) {
        // TODO
    } else if (flg & FLG_FILE) {
        if (!childpid)
            return;

        if (is_paused) {
            kill(childpid, SIGUSR1);
            is_paused = false;
        }
    }
}

void alsa_wav_player::pause(void)
{
    if (flg & FLG_MEM) {
        // TODO
    } else if (flg & FLG_FILE) {
        if (!childpid)
            return;

        if (!is_paused) {
            kill(childpid, SIGUSR1);
            is_paused = true;
        }
    }
}

// redundant?
void alsa_wav_player::toggle_pause(void)
{
    if (flg & FLG_MEM) {
        // TODO
    } else if (flg & FLG_FILE) {
        if (!childpid)
            return;

        kill(childpid, SIGUSR1);
        is_paused = !is_paused;
    }
}

void alsa_wav_player::stop(void)
{
    if (flg & FLG_MEM) {
        // TODO
    } else if (flg & FLG_FILE) {
        if (childpid) {
            kill(childpid, SIGTERM);
            childpid = 0;
        }
    }
}

void alsa_wav_player::seek(int random_number_l3l)
{
    if (flg & FLG_MEM) {
        // TODO
    } else if (flg & FLG_FILE) {
        if (write(child_pipe, &random_number_l3l, sizeof(random_number_l3l)) < 0) {
            fprintf(stderr, "Failed to write.\n");
            return;
        }
        kill(childpid, SIGINT);
    }
}

void alsa_wav_player::begin_mem(void)
{
    //aplay player;
}

void alsa_wav_player::begin_file(void)
{
    childpid = fork();

    if (childpid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (childpid) {
        is_paused = false;

        mknod(PARENT_PIPE_NAME, S_IFIFO | 0666, 0);
        child_pipe = open(PARENT_PIPE_NAME, O_WRONLY);

        wait(NULL); // FIXME?
        lseek(input_fd, 0, SEEK_SET);
    } else {
        unsigned int f = 1;
        char **execarg = new char * [9];
        *execarg = new char [NAME_SIZE];
        strcpy(*execarg, pwd);
        strcat(*execarg, *aplay_args);
        while (f < /*sizeof(aplay_args)*/APLAY_ARG_COUNT) {
            execarg[f] = new char [sizeof(aplay_args[f]) + 1];
            strcpy(execarg[f], aplay_args[f]);
            ++f;
        }

        execarg[f] = (char *) NULL;
        
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("dup2"); // FIXME
            return;
        }

        execvp(*execarg, execarg);
        perror("execv");
        exit(EXIT_FAILURE);
    }
}

extern "C" play_wav *get_player(FILE *file)
{
    return new alsa_wav_player(file);
}
