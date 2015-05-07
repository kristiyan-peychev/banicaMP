#ifndef SONG_H
#define SONG_H

#include <cstring>
#include <cstdio>
#include <exception>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#include "../../decode/decoder.h"
#include "../../decode/get.h"
#include "../play/play.h"

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


	song_info(const char* path)
	{

		TagLib::FileRef f(path);
        
        if(f.isNull() || !f.tag() || !f.audioProperties()){
            throw tags_missing_exception();
        }

		TagLib::AudioProperties* properties = f.audioProperties();
		TagLib::Tag* tag = f.tag();
        title = tag->title();
        artist = tag->artist();
        album = tag->artist();
        year = tag->year();
        genre = tag->genre();
        bitrate = properties->bitrate();
        sample_rate = properties->sampleRate();
        channels = properties->channels();
        length = properties->length();
	}
};


class song{
private:
  	song_info info;
	char* path;
	const char* encoding;
	decoder* dec;
    FILE* decoded_file;
    FILE* f;
    play_wav* player;

    void decode_song();
    void clear_song();

public:
	song(const char*);
	~song();
    

    const song_info& get_info() const; 
	const char* get_path() const;
    const char* get_encoding() const;


    void start();
    void pause();
    void stop();
};

#endif
