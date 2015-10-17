#include "play.h"

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

static const char *aplay_args[] = {
	"/aplay", // executable name
    "-f", "cd", // 16-bit signed integer
    "-t", "wav", // WAVE file
    "-q", // quiet mode
    "-" // read from stdin
};

alsa_wav_player::alsa_wav_player(FILE *filep) :
        is_paused(true), childpid(0), child_pipe(0)/*, filedsc(fileno(filep))*/,
        /*mem(NULL), */flg(FLG_FILE)
{
    printf("alsa_wav_player: filep:%p\n", filep);
    filedsc = fileno(filep);
    //mknod(PARENT_PIPE_NAME, S_IFIFO | 0666, 0);
    //child_pipe = open(PARENT_PIPE_NAME, O_WRONLY);
}

//alsa_wav_player::alsa_wav_player(memory *m) :
        //is_paused(true), filedsc(-1), childpid(0), mem(m), flg(FLG_MEM)
//{ }

alsa_wav_player::alsa_wav_player(int fd) :
        is_paused(true), child_pipe(0), childpid(0), filedsc(fd)/*, mem(NULL)*/,
        flg(FLG_FILE)
{ }

alsa_wav_player::~alsa_wav_player(void)
{
    stop();
    close(child_pipe);
}

void alsa_wav_player::begin(void)
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
        if (flg & FLG_FILE)
            lseek(filedsc, 0, SEEK_SET);
	} else {
		unsigned int f = 1;
		char **execarg = new char * [9];
		*execarg = new char [NAME_SIZE];
		strcpy(*execarg, pwd);
		strcat(*execarg, *aplay_args);
		while (f < /*sizeof(aplay_args)*/7) {
			execarg[f] = new char [sizeof(aplay_args[f]) + 1];
			strcpy(execarg[f], aplay_args[f]);
			++f;
		}
        if (flg & FLG_MEM) {
            // read from shared mem flag
            execarg[f] = new char [2];
            strcpy(execarg[f], "a");
            ++f;
        }
		execarg[f] = (char *) NULL;

		if ((flg & FLG_FILE) && dup2(filedsc, STDIN_FILENO) == -1) {
			perror("dup2");
			exit(EXIT_FAILURE);
		}

		execvp(*execarg, execarg);

		perror("execv");
		exit(EXIT_FAILURE);
	}
}

void alsa_wav_player::play(void)
{
	if (!childpid)
		return;

	if (is_paused) {
		kill(childpid, SIGUSR1);
		is_paused = false;
    }
}

void alsa_wav_player::pause(void)
{
	if (!childpid)
		return;

	if (!is_paused) {
		kill(childpid, SIGUSR1);
		is_paused = true;
	}
}

// redundant?
void alsa_wav_player::toggle_pause(void)
{
	if (!childpid)
		return;

	kill(childpid, SIGUSR1);
	is_paused = !is_paused;
}

void alsa_wav_player::stop(void)
{
	if (childpid) {
		kill(childpid, SIGTERM);
        childpid = 0;
    }
}

void alsa_wav_player::seek(int random_number_l3l)
{
    if (write(child_pipe, &random_number_l3l, sizeof(random_number_l3l)) < 0) {
        fprintf(stderr, "Failed to write.\n");
        return;
    }
    kill(childpid, SIGINT);
}

extern "C" play_wav *get_player(FILE *file)
{
	return new alsa_wav_player(file);
}

