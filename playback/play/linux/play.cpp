#include "play.h"

#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <exception>
//#include <thread>

static void throw_end_of_song(int) throw()
{
	throw playbackend_except();
}

alsa_wav_player::alsa_wav_player(void) { }

alsa_wav_player::alsa_wav_player(const alsa_wav_player &) { }

alsa_wav_player &alsa_wav_player::operator=(const alsa_wav_player &) { return *this; }

alsa_wav_player::alsa_wav_player(FILE *filep) :
		is_paused(true), filedsc(fileno(filep)), childpid(0)
{ }

alsa_wav_player::alsa_wav_player(int fd) :
		is_paused(true), filedsc(fd), childpid(0)
{ }

alsa_wav_player::~alsa_wav_player(void)
{
	stop();
}

void alsa_wav_player::begin(void)
{
	childpid = fork();

	if (childpid) {
		is_paused = false;
		// LOLDIS
		struct sigaction sa;
		sa.sa_flags = SA_RESTART;
		sa.sa_handler = throw_end_of_song;
		sigfillset(&sa.sa_mask);
		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
				perror("sigaction");
				exit(EXIT_FAILURE);
		}
	} else {
		unsigned int f = 1;
		char **execarg = new char * [8];
		char *pwd = get_current_dir_name();
		*execarg = new char [NAME_SIZE];
		strcpy(*execarg, pwd);
		free(pwd);
		strcat(*execarg, *aplay_args);
		while (f < /*sizeof(aplay_args)*/7) {
			execarg[f] = new char [sizeof(aplay_args[f]) + 1];
			strcpy(execarg[f], aplay_args[f]);
			++f;
		}
		execarg[f] = (char *) NULL;

		if (dup2(filedsc, STDIN_FILENO) == -1) {
			perror("dup2");
			exit(EXIT_FAILURE); // FIXME
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
	if (childpid)
		kill(childpid, SIGKILL);
}

play_wav *get_player(FILE *file)
{
	return new alsa_wav_player(file);
}
