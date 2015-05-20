#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "vector/vector.hpp"
#include "song.h"
#include "tinydir.h"
#include "encodings.h"

class song;

class playlist {
private:
    vector<song*> list;

    void generate(const char*);

public:
    playlist(const char* =NULL, bool=false);
    ~playlist();

    void play_song(int);
    void play_next_song();
    void pause_song(int);
    void stop_song(int);
    void remove_song(int);
    void add_song(int);

    void load(const char*);
    void save(const char*);

    void print_songs();


};

class Observer{
private:
    playlist* list;

public:
    Observer(playlist* l = NULL): list(l){}

    void set_playlist(playlist* l)
    {
        list = l;
    }

    void play_next_song() const
    {
        if(list == NULL)
            printf("END OF SONG!!\n");
        else
            list->play_next_song();
    }


};

extern Observer observer;


#endif
