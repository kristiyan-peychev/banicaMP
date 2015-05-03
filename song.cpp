#include "song.h"
#include "extensions.hpp"

song::song(const char* path)
{
	
	this->path = new char[strlen(path)+1];
	std::strcpy(this->path, path);
	this->info = new song_info(path);
	this->extension = get_file_extension(path);	
}

const char* song::get_path() const
{
	return path;
}

const song_info* song::get_info() const 
{
	return info;
}

song::~song()
{
	delete[] path;
	delete info;
}

int ffdf(){
	return 42;
}

const char* song::get_extension() const
{
	return this->extension;
}
