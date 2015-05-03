#include "../song.h"
#include "../../vector/vector.h"


class playlist{
private:
    vector<song*> list;

    void generate(char*);

public:
    playlist(char*, bool);

    void play_song(int);
    void pause_song(int);
    void stop_song(int);
    void remove_song(int);
    void add_song(int);

    void load(char*);
    void save(char*);


};
