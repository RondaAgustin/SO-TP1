#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#define MAX_STRING_LENGTH 256
#define MD5_LENGTH 32  

// Forks
#define SLAVE_FILE_NAME "./slave\0"

//Semaphore
#include <semaphore.h>

#define SHM_SEM_NAME "sem_shm" // nombre de memoria compartida para el semaforo
//---------

// Shared Memory
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define ASCII_EOF 4
#define MEMORY_CHUNK 86
#define CONECT_TIME 2

int read_shm(char* data, char* buff, int* flag);        // Devuelve el largo del string guardado, data siempre tendra espacio
int open_shm(char* shm_name, int flag, int mode);
void create_shm_space(char* shm_name, int fd, int size);
void * map_shm(int fd, size_t size, int flag);

int share_to_view_process(char *buffer, const char *data);
//---------

// Random
void read_until_end_of_string(int fd, char* buffer, int max_size);

// File
void write_to_result_file(char* buffer);

#define FILE_NAME "resultado.txt"

//---------
#endif