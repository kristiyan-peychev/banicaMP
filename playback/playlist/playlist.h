#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <cstdlib>
#include <ctime>

#include "vector/vector.hpp"
#include "song.h"
#include "tinydir.h"
#include "encodings.h"
#include "../../config/config.h"

class song;

class playlist {
private:
    class decoded_song_handler {
    private:
        config configuration;
        vector<song*> decoded_songs_list;
    public:
        decoded_song_handler(void);
        //decoded_song_handler(const song*, size_t);
        decoded_song_handler(const vector<song*>&);
    public:
        void push(song*) noexcept;
    } song_handler;
private:
    vector<song*> list; //array of all the song
    vector<int> queue;  // array with order of playing
    int size;           // number of songs in playlist
    song* curr_song;    // current song
    int queue_pos;      // current position in queue
    bool playing_now;   //is a song playing
    bool repeat;        //is repeat enabled
    bool shuffle;       // is shuffle enabled
    bool paused;        // is song paused

    void generate(const char*);
    
    bool comp_song(const song&, const song&, char);

public:
    playlist();
    playlist(const char* =NULL, bool=false);
    ~playlist();


    int get_size() const { return size;}
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
