#include "playlist.h"
#include "../encodings.h"

playlist::playlist(const char* path, bool is_playlist)
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
            list.push_back(tmp);
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
        printf("%d: %s\n",i, list[i]->get_info().title);
    }
}

void playlist::load(const char* path)
{

}
