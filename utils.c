// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

void read_until_end_of_string(int fd, char* buffer, int max_size) {
    char byte;
    ssize_t bytes_read;
    size_t total_bytes = 0;

    while (1) {
        bytes_read = read(fd, &byte, 1);

        if(bytes_read == 0) {
            buffer[total_bytes] = '\0';
            return;
        }

        // Evitar desbordamiento del buffer
        if (total_bytes >= max_size) {
            fprintf(stderr, "Buffer overflow\n");
            exit(1);
        }

        buffer[total_bytes++] = byte;

        if (byte == '\0') {
            return;
        }
    }
}

void write_to_result_file(char* buffer) {
    FILE* result_file = fopen(FILE_NAME, "a");
    if(result_file == NULL) {
        perror("Error al abrir el archivo resultado.txt");
        exit(1);
    }
    fprintf(result_file, "%s\n", buffer);
    fclose(result_file);
}

int share_to_view_process(char *buffer, const char *data){
    int i;

    for (i = 0; i < MEMORY_CHUNK - 1 && data[i] != '\0'; i++)
    {
        buffer[i] = data[i];
    }

    buffer[i] = '\0';
    
    i++;

    return i;
}