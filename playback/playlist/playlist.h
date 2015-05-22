#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <cstdlib>
#include <ctime>

#include "vector/vector.hpp"
#include "song.h"
#include "tinydir.h"
#include "encodings.h"

class song;

class playlist {
private:
    vector<song*> list;
    vector<int> queue;
    int size;
    int curr_song;
    int queue_pos;
    bool playing_now;
    bool repeat;
    bool shuffle;

    void generate(const char*);
    
    bool comp_song(const song&, const song&, char);

public:
    playlist(const char* =NULL, bool=false);
    ~playlist();

    void play_song(int);
    void play_next_song();
    void pause_song();
    void stop_song();
    void seek(int);

    void remove_song(int);
    void add_song(const char*);
    void add_song(song*);

    void load(const char*);
    void save(const char*);

    void print_songs();
    void shuffle_list();

    void toggle_repeat();
    void toggle_shuffle();


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
