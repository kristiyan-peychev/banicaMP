#include "../../vector/vector.hpp"
#include "song.h"
#include "tinydir.h"
#include "encodings.h"


class playlist{
private:
    vector<song*> list;

    void generate(const char*);

public:
    playlist(const char* =NULL, bool=false);
    ~playlist();

    void play_song(int);
    void pause_song(int);
    void stop_song(int);
    void remove_song(int);
    void add_song(int);

    void load(const char*);
    void save(const char*);

    void print_songs();


};
