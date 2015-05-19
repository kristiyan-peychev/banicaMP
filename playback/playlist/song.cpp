#include "song.h"
#include "encodings.h"
#include "sys/wait.h"

song::song(const char* p): info(p),path(NULL), dec(NULL), decoded_file(NULL),
song_file(NULL), player(NULL)
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

void song::load_song()
{
    if(dec == NULL){
        song_file = fopen(path, "r+b");
        dec = get_decoder(song_file, encoding);
    }

    if(player == NULL){
        if(decoded_file == NULL){
            tmpnam(tmp_file_name);
            printf("%s\n", tmp_file_name);
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
    
    //if(song_file != NULL)
      //  fclose(song_file);
    if(player != NULL)
        delete player;
    
    player = NULL;
    decoded_file = NULL;
    song_file = NULL;
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

void start_thread(song& s){
    s.player->begin();
    s.clear_song();
    //throw end_of_song_exception();
}

void song::start()
{
    if(player == NULL)
        load_song();

    t = new std::thread(&start_thread, std::ref(*this));
    t->detach();
    printf("%d fff\n",t->joinable());
}

void song::pause()
{
    //if(player == NULL)
        //throw player_not_found_exception();
        player->toggle_pause();
}

void song::stop()
{
    //if(player == NULL)
        //throw player_not_found_exception();
        player->stop();
}

