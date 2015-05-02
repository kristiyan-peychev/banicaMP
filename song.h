#ifndef SONG_H
#define SONG_H

#include <cstring>
#include <cstdio>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

#include "decode/decoder.h"

struct song_info{
	TagLib::Tag *tag;
	TagLib::AudioProperties *properties;

	song_info(const char* path)
	{
		TagLib::FileRef f(path);
		tag = f.tag();
		properties = f.audioProperties();
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
	const char* get_path() const;
    const char* get_extension() const;
	const song_info* get_info() const;
};

#endif
