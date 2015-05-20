#include "../playback/playlist/playlist.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

int main(){
    playlist p("/media/win/Music/Avengers OST");
    observer.set_playlist(&p);
    p.shuffle();
    p.print_songs();
    char command[20];
    int a,b;
    while(true){
        printf("> ");
        scanf("%s",command);
        if(!strcmp(command, "play")){
            scanf("%d", &a);
            p.play_song(a);
        } else if(!strcmp(command, "pause")){
            p.pause_song();
        }
    }

}
