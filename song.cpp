#include "song.h"
    
const char* song::Signatures[song::NumOfSignatures] = {"fLaC","˙ű", "ID3", "RIFF....WAVE"}; 
const int song::Sizes[song::NumOfSignatures] = {4, 2, 3, 12};
const char* song::Extensions[song::NumOfSignatures] = {"FLAC", "MP3", "MP3", "WAV"};


song::song(const char* path)
{
	
	this->path = new char[strlen(path)+1];
    std::strcpy(this->path, path);
	this->info = new song_info(path);
    this->extension = get_file_extension();	
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

const char* song::get_file_extension()
{
    FILE* f = fopen(this->path, "r");
    char buff[13];
    fread(buff, sizeof(char), 13, f);
    fclose(f);
    for(int i = 0; i < NumOfSignatures; i++){
        if(strncmp(buff, Signatures[i], Sizes[i]) == 0)
            return Extensions[i];
    }
    return "UNKNOWN";

}

const char* song::get_extension() const
{
    return this->extension;
}
