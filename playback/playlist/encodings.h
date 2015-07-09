#ifndef ENCODINGS_H
#define ENCODINGS_H

#include <cstdio>
#include <cstring>

#include "../../decode/get.h"

static const int NUMSIGNATURES = 3;
static const char* SIGNATURES[NUMSIGNATURES] = {"fLaC","˙ű", "ID3"}; 
static constexpr int SIZES[NUMSIGNATURES] = {4, 2, 3};
static constexpr auto get_max_size(void) {
    int ret = SIZES[0];
    int n = 1;
    while (n < NUMSIGNATURES) {
        ret = ret < SIZES[n] ? SIZES[n] : ret;
        n++;
    }
    return ret;
}
static constexpr int MAXSIZE = get_max_size();
static const enum encodings ENCODINGS[NUMSIGNATURES] = {ENC_FLAC, ENC_MP3, ENC_MP3};

inline const enum encodings get_file_encoding(const char* path)
{
    FILE* f = fopen(path, "r");
    char buff[MAXSIZE+1];
    fread(buff, sizeof(char), MAXSIZE, f);
    fclose(f);
    for(int i = 0; i < NUMSIGNATURES; i++){
        if(strncmp(buff, SIGNATURES[i], SIZES[i]) == 0)
            return ENCODINGS[i];
    }
    return ENC_UNK;
}

#endif
