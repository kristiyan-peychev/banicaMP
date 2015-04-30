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
    //I know naming sux
    static const int NumOfSignatures = 4;
    static const char* Signatures[];
    static const char* Extensions[];
    static const int Sizes[];


	song_info* info;
	char* path;
	const char* extension;
	decoder* dec;

    const char* get_file_extension();
public:
	song(const char*);
	~song();
	const char* get_path() const;
    const char* get_extension() const;
	const song_info* get_info() const;
};

#endif
