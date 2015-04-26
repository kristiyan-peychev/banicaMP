#ifndef SONG_H
#define SONG_H

#include <string>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

struct song_info{
	TagLib::Tag *tag;
	TagLib::AudioProperties *properties;

	song_info(std::string path)
	{
		TagLib::FileRef f(path.c_str());
		tag = f.tag();
		properties = f.audioProperties();
	}
};

class song{
private:
	song_info* info;
	std::string path;
public:
	song(std::string);
	~song();
	std::string get_path() const;
	const song_info* get_info() const;
};

#endif
