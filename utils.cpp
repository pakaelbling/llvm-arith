#include <cstdio>
#include <iostream>
#include "utils.h"

namespace utils{
    char *readFile(char* filePath) {
        // open the file
        FILE *fp = fopen(filePath, "rb");
        if (!fp) {
            fputs("File read at provided path failed", stderr);
            exit(1);
        }
        // figure out the size of buffer we need to allocate
        fseek(fp, 0l, SEEK_END);
        long buffSize = ftell(fp);
        rewind(fp);
        char* buff = (char*)calloc(1, buffSize+1);
        if(!buff){
            fclose(fp);
            fputs("memory alloc failed", stderr);
            exit(1);
        }
        // read the file into the created buffer and close the file
        fread(buff, buffSize, 1, fp);
        fclose(fp);
        return buff;
        // NOTE: We didn't free the allocated buffer! The client is responsible for that
    }
} // namespace utils
