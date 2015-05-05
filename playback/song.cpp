#include "song.h"
#include "encodings.h"

song::song(const char* path): info(path)
{
	
	this->path = new char[strlen(path)+1];
	std::strcpy(this->path, path);
    FILE* f = fopen(this->path, "r");
	this->encoding = get_file_encoding(path);	
    //this->dec = get_decoder(f,this->encoding); 
}

const char* song::get_path() const
{
	return path;
}

const song_info& song::get_info() const 
{
	return info;
}

song::~song()
{
	delete[] path;
}


const char* song::get_encoding() const
{
	return this->encoding;
}
