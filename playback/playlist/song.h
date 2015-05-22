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


class player_not_found_exception : public std::exception {
public:
    virtual const char* what() const throw()
    {
        return "Player does not exist";
    }
};

class tags_missing_exception : public std::exception {
public:
    virtual const char* what() const throw()
    {
        return "Cannot read tags";
    }
};
class end_of_song_exception : public std::exception {
public:
    virtual const char* what() const throw()
    {
        return "End of song";
    }
};

struct song_info{
    TagLib::String title;
    TagLib::String artist;
    TagLib::String album;
    TagLib::String genre;
    int year;
    int bitrate;
    int sample_rate;
    int channels;
    int length;


	song_info(const char* path):title(path), artist(""), album(""), genre(""),
            year(0), bitrate(0), sample_rate(0), channels(1), length(0)
	{

		TagLib::FileRef f(path);

        if (f.isNull())
            perror("Cannot read tags\n");
        else{
            if(f.tag()){
                TagLib::Tag* tag = f.tag();
                title = tag->title();
                artist = tag->artist();
                album = tag->artist();
                year = tag->year();
                genre = tag->genre();
            }

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
  	song_info info;
	char* path;
	const char* encoding;
	decoder* dec;
    FILE* decoded_file;
    FILE* song_file;
    play_wav* player;
    std::thread* t;
    bool manual_stop;

    void load_song();
    void clear_song();

public:
	song(const char* );
	~song();
    

    const song_info& get_info() const; 
	const char* get_path() const;
    const char* get_encoding() const;


    void start();
    void pause();
    void stop();
    void seek(int);

    friend void start_thread(song&);
};

#endif
