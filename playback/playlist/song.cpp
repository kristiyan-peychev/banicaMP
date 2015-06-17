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
        song_file = fopen(path, "r");
        dec = get_decoder(song_file, encoding);
    }

    //decode the music file
    if(decoded_file == NULL){
        //create temp file to hold the decoded song
        decoded_file = tmpfile();
        dec->decode(decoded_file); 

        //switch to read and return pos indicator to beginning
        rewind(decoded_file);
    }

    //load wav player
    if(player == NULL){
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
    if(song_file != NULL)
        fclose(song_file);

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


void play_song(song& s)
{
    printf("%s\n", s.info.title.toCString(true));
    
    s.load_song();

    s.manual_stop = false;
   
    //start the song
    s.player->begin();
    //clear song after it finishes
    s.clear_song();

    //tell playlist to play next song
    if(!s.manual_stop)
        observer.play_next_song();

    printf("end of thread\n");
}

void song::start()
{
    //start the song in a new thread
    t = std::thread(play_song, std::ref(*this));

    t.detach();
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
