#include <stdio.h>

// Shared Memory
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */


#define READ_LENGTH 86
// File: {20 chars}.txt | MD5: {32 chars} | PID: {5 chars}\0

// Devuelve el largo del string guardado
// Data siempre tendra espacio
int read_shm(char* data, char* buff, int* flag);

int main(int argc, char** argv){
    int flag = 0;


    if (argc < 2){
        printf("Usage: %s shared_memory", argv[0]);
        return -1;
    }

    char * shm_name = argv[1];

    int fd = shm_open(shm_name, O_RDONLY);

    if (fd == -1){
        printf("Cannot open %s to read.\n", shm_name);
        return -1;
    }

    char* buffer = mmap(NULL, READ_LENGTH + 1, PROT_READ, MAP_SHARED, fd, 0);

    if (buffer == MAP_FAILED){
        printf("Cannot map shared memory.\n");
        return -1;
    }








    munmap(buffer, sizeof(char) * READ_LENGTH + 1);
    shm_unlink(shm_name);

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
