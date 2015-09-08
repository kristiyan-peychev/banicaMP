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
    vector<song *> list;            //array of all the song
    vector<song *> queue;           // array with order of playing
    vector<song *> priority_queue;  // array to specify songs 
    size_t pq_pos;                  // to be played before the queue
    size_t queue_pos;               // current position in queue
    int size;                       // number of songs in playlist
    song* curr_song;                // current song
    bool playing_now;               //is a song playing
    bool repeat;                    //is repeat enabled
    bool shuffle;                   // is shuffle enabled
    bool paused;                    // is song paused

    void generate(const char*);
public:
    playlist();
    playlist(const char* =NULL, bool=false);
    ~playlist();


    int get_size() const { return list.size();}
    void play_song(size_t);
    void play_song(song *);
    void play_next_song();
    void pause_song();
    void stop_song();
    void seek(int);

    void enqueue_prioriy(song *);
    void remove_priority(song *);
    void remove_priority(size_t idx);
    void clear_priority();

    void remove_song(size_t);
    void remove_song(song *);
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
