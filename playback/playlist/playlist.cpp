#include "playlist.h"

Observer observer;

playlist::playlist(const char* path, bool is_playlist): size(0), curr_song(0),
        queue_pos(0), playing_now(false), repeat(false), shuffle(false)
{
    if(path){
        if(is_playlist)
            load(path);
        else
            generate(path);
    }
}

playlist::~playlist()
{
    int size = list.size();
    for(int i = 0; i < size; i++)
        delete list[i];
}

void playlist::generate(const char* path)
{
    //NEEDS MORE ERROR HANDLING
    tinydir_dir dir;
    tinydir_open(&dir, path);
    while(dir.has_next){
        tinydir_file file;

        if(tinydir_readfile(&dir, &file) == -1){
            perror("Error reading file\n");
            tinydir_next(&dir);
            continue;
        }

        if( !file.is_dir && strcmp(get_file_encoding(file.path), "UNKNOWN") ){
            add_song(file.path);
                    }
        
        if(file.is_dir && strcmp(file.name, ".") && strcmp(file.name, ".."))
            generate(file.path);
        tinydir_next(&dir);

    }
    tinydir_close(&dir);
}

void playlist::print_songs()
{
    for(int i = 0; i < list.size(); i++){
        printf("%d: %s %d\n",i, list[i]->get_info().title.toCString(true), list[i]->get_info().length);
    }

    for(int i = 0; i < queue.size(); i++)
        printf("%d ", queue[i]);
    printf("\n");
}

void playlist::load(const char* path)
{

}

void playlist::save(const char* path)
{
}

void playlist::play_song(int pos)
{
    if(playing_now)
       stop_song();
    curr_song = pos;
    queue_pos = queue.find(pos);
    list[curr_song]->start();
    playing_now = true;
    
}

void playlist::play_next_song()
{
    if(playing_now)
        stop_song();
    if(!repeat && queue_pos == size -1){
        printf("end of playlist\n");
        return;
    }
    queue_pos = (queue_pos + 1) % size;
    curr_song = queue[queue_pos];
    play_song(curr_song);
}

void playlist::pause_song()
{
    if(playing_now)
        list[curr_song]->pause();
}

void playlist::stop_song()
{
    if(playing_now){
        list[curr_song]->stop();
        playing_now = false;
    }
}
//bugged
void playlist::seek(int secs)
{
    if(playing_now)
        list[curr_song]->seek(secs);
}

void playlist::remove_song(int pos)
{
    int idx = queue.find(pos);
    if(idx == -1)
        perror("Item not in playlist!\n");
    else{
        list.remove(pos);
        queue.remove(idx);
        size--;
    }
}

void playlist::add_song(const char* path)
{
    song* new_song = new song(path);
            //don't add songs without metadata
    if(new_song->get_info().length != 0){
        list.push_back(std::ref(new_song));
        queue.push_back(size++);
    }
}

void playlist::add_song(song* s)
{
    list.push_back(s);
    queue.push_back(size++);
}

void playlist::shuffle_list()
{
    srand(time(NULL));
    for(size_t i = size - 1; i >0; i--){
        size_t r = rand() % size;
        std::swap(queue[i], queue[r]);
    }
}

void playlist::toggle_repeat()
{
    repeat = !repeat;
}

void playlist::toggle_shuffle()
{
    if(shuffle){
        shuffle = false;
        queue.sort();
    }
    else{
        shuffle = true;
        shuffle_list();
    }
}

bool playlist::comp_song(const song& s1, const song& s2, char ch )
{
    //can be done better, but i don't give a fuck
    switch (ch){
        case 't' : return s1.get_info().title < s2.get_info().title; break;
        case 'a' : return s1.get_info().artist < s2.get_info().artist; break;
        case 'l' : return s1.get_info().length < s2.get_info().length; break;
        default : return s1.get_info().title < s2.get_info().title; break;
    }

}
