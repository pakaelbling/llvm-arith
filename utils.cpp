#include <cstdio>
#include <iostream>
#include "utils.h"

namespace utils{
    char *readFile(char* filePath) {
        FILE *fp = fopen(filePath, "rb");
        if (!fp) {
            fputs("File read at provided path failed", stderr);
            exit(1);
        }
        fseek(fp, 0l, SEEK_END);
        long buffSize = ftell(fp);
        rewind(fp);
        char* buff = (char*)calloc(1, buffSize+1);
        if(!buff){
            fclose(fp);
            fputs("memory alloc failed", stderr);
            exit(1);
        }
        fread(buff, buffSize, 1, fp);
        fclose(fp);
        return buff;
    }
} // namespace utils
