#include "../playback/playlist/playlist.h"
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void print_help(){
    printf("Use one of the following commands:\n");
    printf("list - lists all songs\n");
    printf("play <song_number> - play the song with number song_number\n");
    printf("pause - pauses current song\n");
    printf("seek  <num_secs> - go forward(backward) by num_secs\n");
    printf("stop - stops current song\n");
    printf("shuffle - toggles shuffle\n");
    printf("repeat - toggles repeat\n");
    printf("quit - Exit the program\n");
}

int main(){
    playlist p("/media/win/Downloads/Music/OST.Avengers.Assemble.VA.2012.MP3.320kbps - SMG/");
    observer.set_playlist(&p);
    p.print_songs();
    char command[20];
    int a;
    while(true){
        try{
        printf("\n> ");
        scanf("%s",command);
        if(!strcmp(command,"list"))
            p.print_songs();
        else if(!strcmp(command, "play")){
            scanf("%d", &a);
            p.play_song(a);
        }else if(!strcmp(command, "pause"))
            p.pause_song();
        else if(!strcmp(command, "seek")){
            scanf("%d", &a); 
            p.seek(a);
        }else if(!strcmp(command, "stop"))
            p.stop_song();
        else if(!strcmp(command, "shuffle"))
            p.toggle_shuffle();
        else if(!strcmp(command, "repeat"))
            p.toggle_repeat();
        else if(!strcmp(command, "help"))
            print_help();
        else if(!strcmp(command, "quit"))
            break;
        } catch(...){
            printf("Exception\n");
            p.stop_song();
        }
    }

}
