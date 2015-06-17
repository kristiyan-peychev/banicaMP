#ifndef SONG_H
#define SONG_H

#include <cstring>
#include <cstdio>
#include <exception>
#include <thread>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#include "../../decode/decoder.h"
#include "../../decode/get.h"
#include "../play/play.h"
#include "playlist.h"

#define BYTES_PER_SEC 44100 * 2 * 2

struct song_info{
    TagLib::String title;  // song title
    TagLib::String artist; // song artist
    TagLib::String album;  // song album
    TagLib::String genre;  // song genre
    int year;              // song year
    int bitrate;           // song bitrate
    int sample_rate;       // song sample_rate
    int channels;          // song channels
    int length;            // song length


	song_info(const char* path):title(path), artist(""), album(""), genre(""),
            year(0), bitrate(0), sample_rate(0), channels(1), length(0)
	{
    
		TagLib::FileRef f(path);

        if (f.isNull())
            perror("Cannot read metadata\n");
        else{
            //read tags
            if(f.tag()){
                TagLib::Tag* tag = f.tag();
                title = tag->title();
                artist = tag->artist();
                album = tag->artist();
                year = tag->year();
                genre = tag->genre();
            }
            //read audio properties
            if(f.audioProperties()){
                TagLib::AudioProperties* properties = f.audioProperties();
                bitrate = properties->bitrate();
                sample_rate = properties->sampleRate();
                channels = properties->channels();
                length = properties->length();
            }
        }
    }
};


class song{
private:
  	song_info info;       // song info
	char* path;           // file path
	const char* encoding; // file encoding
	decoder* dec;         //song decoder
    FILE* decoded_file;   //decoded wav file
    FILE* song_file;      //song file
    play_wav* player;     // wav player
    std::thread t;        //thread to play the song
    bool manual_stop;     // indicates if user stopped song


public:
	song(const char* );
	~song();
    

    void load_song();
    void clear_song();

    const song_info& get_info() const; 
	const char* get_path() const;
    const char* get_encoding() const;


    void start();
    void pause();
    void stop();
    void seek(int);

    friend void play_song(song&);
};

#endif
