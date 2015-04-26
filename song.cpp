#include "song.h"

song::song(std::string path)
{

	this->path = path;
	info = new song_info(path);
}

std::string song::get_path() const
{
	return path;
}

const song_info* song::get_info() const 
{
	return info;
}

song::~song()
{
	delete info;
}
