#include <cstdio>
#include <cstring>
#include "../playback/playlist/tinydir.h"
#include "dirent.h"
#include "../playback/encodings.h"


void test(const char* path){
    tinydir_dir dir;
    tinydir_open(&dir, path);
    printf("dir: %s\n", dir.path);
    while(dir.has_next){
        tinydir_file file;
        if(tinydir_readfile(&dir, &file)== -1){
            perror("Error");
            return;
        }
        if(!strcmp(file.name, ".") || !strcmp(file.name, ".."))
           continue;

        if(strcmp(get_file_extension(file.path), "UNKNOWN"))
            printf("%s\n", file.name);
        if(file.is_dir && strcmp(file.name, ".") && strcmp(file.name, "..")){
            test(file.path);
        }
        tinydir_next(&dir);
    }
    tinydir_close(&dir);
}


int main(){
    test("/home/nikolay/");
    return 0;
}
