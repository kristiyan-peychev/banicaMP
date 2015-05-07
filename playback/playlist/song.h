#ifndef SONG_H
#define SONG_H

#include <cstring>
#include <cstdio>
#include <stdexcept>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#include "../../decode/decoder.h"
#include "../../decode/get.h"
#include "../play/play.h"



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
            //TODO
           throw std::out_of_range("blah2"); 
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
    play_wav* player;


public:
	song(const char*);
	~song();
    

    const song_info& get_info() const; 
	const char* get_path() const;
    const char* get_encoding() const;

    void decode_song();
    void clear_song();

    void start();
    void pause();
    void stop();
};

#endif
