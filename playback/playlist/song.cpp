#include "song.h"
#include "encodings.h"

song::song(const char* p): info(p),path(NULL), dec(NULL), decoded_file(NULL),f(NULL), player(NULL)
{
	
	path = new char[strlen(p)+1];
	std::strcpy(path, p);
	encoding = get_file_encoding(path);

    
}

song::~song()
{
    if(path != NULL)
	    delete[] path;
    clear_song();
}

void song::decode_song()
{
    if(dec == NULL){
        f = fopen(path, "rb");
        dec = get_decoder(f, encoding);
    }

    if(player == NULL){
        if(decoded_file == NULL){
            tmpnam(tmp_file_name);
            decoded_file = fopen(tmp_file_name, "w+b");;
            dec->decode(decoded_file);
            fclose(decoded_file);
        }

        decoded_file = fopen(tmp_file_name, "r+b");
        player = get_player(decoded_file);
    }
}

void song::clear_song()
{
    if(decoded_file != NULL){
        fclose(decoded_file);
        remove(tmp_file_name);
    }
    if(dec != NULL)
        delete dec;
    //CAUSES DOUBLE FREE OR CORRUPTION
    //WTF
   // if(f != NULL)
     //   fclose(f);

    if(player != NULL)
        delete player;
    
    player = NULL;
    decoded_file = NULL;
    f = NULL;
    dec = NULL;
    
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
        printf("4\n");
        #ifdef PLAY_J01D18YC
        printf("5\n");
        wait(NULL);
        #endif
        printf("ffdf\n");
    } catch ( playbackend_except &e){
        //NO IDEA WHAT TO DO HERE YET
        printf("ffffff\n");
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
    //TODO
    clear_song();
}

