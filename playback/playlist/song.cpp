#include "song.h"
#include "encodings.h"

song::song(const char* path): info(path), decoded_file(NULL), player(NULL)
{
	
	this->path = new char[strlen(path)+1];
	std::strcpy(this->path, path);
    FILE* f = fopen(this->path, "r");
	this->encoding = get_file_encoding(path);	
    this->dec = get_decoder(f,this->encoding); 
    
}

void song::decode_song()
{
    if(decoded_file == NULL){
        decoded_file = tmpfile();
        dec->decode(decoded_file);
        player = get_player(decoded_file);
        
    }
}

void song::clear_song()
{
    if(decoded_file != NULL){
        fclose(decoded_file);
        delete player;
        player = NULL;
        decoded_file = NULL;
    }
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

void song::start()
{
    if(player == NULL)
        decode_song();

    player->play();

}

void song::pause()
{
    if(player == NULL)
        //MUST LEARN HOW TO CREATE EXCEPTIONS!!!!!
        throw std::out_of_range("blah3");
    player->pause();
}

void song::stop()
{
    if(player == NULL)
        throw std::out_of_range("blah4");
    player->stop();
    clear_song();
}

