#include "playlist.h"

Observer observer;

playlist::playlist(const char* path, bool is_playlist): size(0), curr_song(0),
        queue_pos(0), playing_now(false)
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

        if(strcmp(get_file_encoding(file.path), "UNKNOWN") ){
            song* tmp = new song(file.path);
            s = tmp;
            list.push_back(std::ref(tmp));
            queue.push_back(size++);
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
        printf("%d: %s\n",i, list[i]->get_info().title.toCString(true));
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
// FROM HERE ON THE CODE IS STUPID
// PLEASE DON'T JUDGE ME (MUCH)
void playlist::play_song(int pos)
{
    stop_song();
    curr_song = pos;
    list[curr_song]->start();
    
}

void playlist::play_next_song()
{
    curr_song = queue[queue_pos++];
    play_song(curr_song);
}

void playlist::pause_song()
{
    if(playing_now)
        list[curr_song]->pause();
}

void playlist::stop_song()
{
    if(playing_now)
        list[curr_song]->pause();
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
    list.push_back(new_song);
    queue.push_back(size++);
}

void playlist::add_song(song* s){
    list.push_back(s);
    queue.push_back(size++);
}

void playlist::shuffle(){
    srand(time(NULL));
    for(size_t i = size - 1; i >0; i--){
        size_t r = rand() % size;
        std::swap(queue[i], queue[r]);
    }
    curr_song = queue[0];
}
