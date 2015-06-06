#include "song.h"
#include "encodings.h"
#include "sys/wait.h"


song::song(const char* p): info(p),path(NULL), dec(NULL), decoded_file(NULL),
        song_file(NULL), player(NULL), manual_stop(false)
{
	
	path = new char[strlen(p)+1];
	std::strcpy(path, p);
	encoding = get_file_encoding(path);
    
}

song::~song()
{
    clear_song();
    if(path != NULL)
	    delete[] path;
}

void song::load_song()
{
    //load decoder
    if(dec == NULL){
        song_file = fopen(path, "rb");
        dec = get_decoder(song_file, encoding);
    }

    //load wav player
    if(player == NULL){
        if(decoded_file == NULL){
            decoded_file = tmpfile();
            dec->decode(decoded_file);

        }
        rewind(decoded_file);
        player = get_player(decoded_file);
    }
}

void song::clear_song()
{
    if(decoded_file != NULL){
        fclose(decoded_file);
    }
    if(dec != NULL)
        delete dec;

    //CAUSES DOUBLE FREE OR CORRUPTION
    //WTF
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

void start_thread(song& s)
{
    s.player->begin();
    s.clear_song();
    if(!s.manual_stop)
        observer.play_next_song();
    printf("end of thread\n");
}

void song::start()
{
    printf("%s\n", info.title.toCString(true));
    load_song();

    manual_stop = false;
    t = new std::thread(start_thread, std::ref(*this));
    t->detach();
}

void song::pause()
{
    if(player != NULL)
        player->toggle_pause();
}

void song::stop()
{
    if(player != NULL){
        manual_stop = true;
        player->stop();
    }
}

void song::seek(int secs)
{
    if(player != NULL){
        player->seek(BYTES_PER_SEC * secs);
    }
}
