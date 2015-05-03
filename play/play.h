#ifndef PLAY_6T2G7RPS

#define PLAY_6T2G7RPS

/* TODO:
 * rewind
 * alter speed
 */

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
	virtual void togle_pause(void) = 0;
	virtual void stop(void) = 0;
private:
	play_wav &operator=(const play_wav &) { return *this; }
	play_wav(const play_wav &) { }
};

#endif /* end of include guard: PLAY_6T2G7RPS */
