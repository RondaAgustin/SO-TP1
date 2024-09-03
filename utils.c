#include "utils.h"

int read_shm(char* data, char* buff, int* flag){
    int i;

    for (i = 0; i < MEMORY_CHUNK - 1 && buff[i] != 0 && buff[i] != ASCII_EOF ; i++)
    {
        data[i] = buff[i];
    }

    data[i] = '\0';

    if (buff[i] == ASCII_EOF)
    {
        *flag = -1;
    }

    i++;

    return i;
}

int open_shm(char* shm_name, int flag, int mode){

    int fd = shm_open(shm_name, flag, mode);

    if (fd == -1){
        printf("Cannot open %s to read.\n", shm_name);
        exit(-1);
    }

    return fd;
}

void create_shm_space(char* shm_name, int fd, int size){
    if (ftruncate(fd, sizeof(char) * size) == -1){      // reservamos el espacio necesario para la shm
        shm_unlink(shm_name);       // remove shared memory object name
        printf("Cannot create space for shared memory.\n");
        exit(-1);
    }
}

void * map_shm(int fd, size_t size, int flag){
    void* buffer = mmap(NULL, size, flag, MAP_SHARED, fd, 0);

    if (buffer == MAP_FAILED){
        printf("Cannot map shared memory.\n");
        exit(-1);
    }

    return buffer;
}