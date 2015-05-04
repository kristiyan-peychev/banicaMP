#ifndef SONG_H
#define SONG_H

#include <cstring>
#include <cstdio>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#include "../decode/decoder.h"



struct song_info{
	const char* title;
    const char* artist;
    const char* album;
    int year;
    const char* genre;
    int bitrate;
    int sample_rate;
    int channels;
    int length;


	song_info(const char* path)
	{

		TagLib::FileRef f(path);
        
        if(f.isNull() || !f.tag() || !f.audioProperties()){
            printf("kur\n");
        }

		TagLib::AudioProperties* properties = f.audioProperties();
		TagLib::Tag* tag = f.tag();
        title = tag->title().toCString(true);
        artist = tag->artist().toCString();
        album = tag->artist().toCString();
        year = tag->year();
        genre = tag->genre().toCString();
        bitrate = properties->bitrate();
        sample_rate = properties->sampleRate();
        channels = properties->channels();
        length = properties->length();
	}
};


class song{
private:
  	song_info* info;
	char* path;
	const char* extension;
	decoder* dec;

public:
	song(const char*);
	~song();
    

    const song_info* get_info() const; 
	const char* get_path() const;
    const char* get_extension() const;
};

#endif
