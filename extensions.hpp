#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <cstdio>
#include <cstring>

static const int NumOfSignatures = 4;
static const int maxSize = 12;
const char* Signatures[NumOfSignatures] = {"fLaC","˙ű", "ID3", "RIFF....WAVE"}; 
const int Sizes[NumOfSignatures] = {4, 2, 3, 12};
const char* Extensions[NumOfSignatures] = {"FLAC", "MP3", "MP3", "WAV"};

const char* get_file_extension(const char* path)
{
    FILE* f = fopen(path, "r");
    char buff[maxSize+1];
    fread(buff, sizeof(char), maxSize, f);
    fclose(f);
    for(int i = 0; i < NumOfSignatures; i++){
        if(strncmp(buff, Signatures[i], Sizes[i]) == 0)
            return Extensions[i];
    }
    return "UNKNOWN";

}

#endif
