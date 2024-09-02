#include <stdio.h>


#define READ_LENGTH 86
// File: {20 chars}.txt | MD5: {32 chars} | PID: {5 chars}\0


void read_shm(char* data, int* shared_memory, int* flag);

int main(int argc, char** argv){
    if (argc < 2){
        perror("Usage: ./view shared_memory");
        return -1;
    }









    return 0;
}

void read_shm(char* data, int* shared_memory, int* flag){

}
