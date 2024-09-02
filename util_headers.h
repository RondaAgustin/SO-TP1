#ifndef UTIL_HEADERS
#define UTIL_HEADERS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//Semaphore
#include <semaphore.h>
#define SHM_SEM_NAME "sem_shm" // nombre de memoria compartida para el semaforo

// Shared Memory
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#define ASCII_EOF 26
#define MEMORY_CHUNCK 86

#endif