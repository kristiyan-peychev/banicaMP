#ifndef ENCODINGS_H
#define ENCODINGS_H

#include <cstdio>
#include <cstring>

static const int NUMSIGNATURES = 4;
static const int MAXSIZE = 12;
static const char* SIGNATURES[NUMSIGNATURES] = {"fLaC","˙ű", "ID3", "RIFF....WAVE"}; 
static const int SIZES[NUMSIGNATURES] = {4, 2, 3, 12};
static const char* ENCODINGS[NUMSIGNATURES] = {"FLAC", "MP3", "MP3", "WAV"};

inline const char* get_file_extension(const char* path)
{
    FILE* f = fopen(path, "r");
    char buff[MAXSIZE+1];
    fread(buff, sizeof(char), MAXSIZE, f);
    fclose(f);
    for(int i = 0; i < NUMSIGNATURES; i++){
        if(strncmp(buff, SIGNATURES[i], SIZES[i]) == 0)
            return ENCODINGS[i];
    }
    return "UNKNOWN";

}

#endif
