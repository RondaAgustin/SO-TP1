#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Semaphore
#include <semaphore.h>

#define SHM_SEM_NAME "sem_shm" // nombre de memoria compartida para el semaforo
//---------

// Shared Memory
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define ASCII_EOF 26
#define MEMORY_CHUNK 86

int read_shm(char* data, char* buff, int* flag);        // Devuelve el largo del string guardado, data siempre tendra espacio
int open_shm(char* shm_name, int flag, int mode);
void create_shm_space(char* shm_name, int fd, int size);
void * map_shm(int fd, size_t size, int flag);
//---------

// Random
void read_until_end_of_string(int fd, char* buffer, int max_size);
//---------
#endif