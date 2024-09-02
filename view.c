#include <stdio.h>


#define READ_LENGTH 86
// File: {20 chars}.txt | MD5: {32 chars} | PID: {5 chars}\0

// Devuelve el largo del string guardado
// Data siempre tendra espacio
int read_shm(char* data, char* buff, int* flag);

int main(int argc, char** argv){
    if (argc < 2){
        perror("Usage: ./view shared_memory");
        return -1;
    }









    return 0;
}

int read_shm(char* data, char* buff, int* flag){
    int i;

    for (i = 0; i < READ_LENGTH - 1 && buff[i] != 0 && buff[i] != EOF ; i++)
    {
        data[i] = buff[i];
    }

    data[i] = '\0';

    if (buff[i] == EOF)
    {
        *flag = -1;
    }

    i++;

    return i;
}
