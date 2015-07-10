#ifndef PLAY_6T2G7RPS

#define PLAY_6T2G7RPS

/* TODO:
 * alter speed
 */

#define _LINUX // FIXME
#include "../../memory/memory.h"

#include <cstdio>
#include <exception>

class playbackend_except : public std::exception {
public:
	virtual const char *what(void) const throw()
	{
		return "Playback ended.";
	}
};

class play_wav {
public:
	play_wav(void) { }
	virtual ~play_wav(void) { }
public:
	virtual void begin(void) = 0;
	virtual void play(void) = 0;
	virtual void pause(void) = 0;
	virtual void toggle_pause(void) = 0;
	virtual void stop(void) = 0;
    virtual void seek(int) = 0;
public:
	play_wav &operator=(const play_wav &) = delete;
	play_wav(const play_wav &) = delete;
};

#if defined(linux)
#include "linux/play.h"
#elif defined(WIN32)
#include "windows/play.h"
#endif

extern "C" play_wav *get_player(FILE *);

#endif /* end of include guard: PLAY_6T2G7RPS */
