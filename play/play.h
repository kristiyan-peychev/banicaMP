#ifndef PLAY_6T2G7RPS

#define PLAY_6T2G7RPS

/* TODO:
 * rewind
 * alter speed
 * stop?
 */
class play_wav {
public:
	play_wav(void);
	virtual ~play_wav(void) = 0;
public:
	virtual void begin(void) = 0;
	virtual void play(void) = 0;
	virtual void pause(void) = 0;
	virtual void togle_pause(void) = 0;
private:
	play_wav &operator=(const play_wav &) { }
	play_wav(const play_wav &) { }
};

#endif /* end of include guard: PLAY_6T2G7RPS */
