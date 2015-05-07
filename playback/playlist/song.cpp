#include "song.h"
#include "encodings.h"

song::song(const char* path): info(path), decoded_file(NULL), player(NULL)
{
	
	this->path = new char[strlen(path)+1];
	std::strcpy(this->path, path);
    f = fopen(this->path, "r");
	this->encoding = get_file_encoding(path);	
    this->dec = get_decoder(f,this->encoding); 
    
}

song::~song()
{
	delete[] path;
    fclose(f);
    clear_song();
    if(player != NULL)
        delete player;
    
    if(dec != NULL)
        delete dec;

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
    if(decoded_file != NULL)
        fclose(decoded_file);
    if(player != NULL)
        delete player;
    player = NULL;
    decoded_file = NULL;
    
}

const char* song::get_path() const
{
	return path;
}

const song_info& song::get_info() const 
{
	return info;
}



const char* song::get_encoding() const
{
	return this->encoding;
}

void song::start()
{
    if(player == NULL)
        decode_song();
    try{
    player->begin();
    #ifdef PLAY_J01D18YC
        wait(NULL);
    #endif
    } catch ( playbackend_except &e){
        //NO IDEA WHAT TO DO HERE YET
    }
    

}

void song::pause()
{
    if(player == NULL)
        throw player_not_found_exception();
    player->toggle_pause();
}

void song::stop()
{
    if(player == NULL)
        throw player_not_found_exception();
    player->stop();
    clear_song();
}

