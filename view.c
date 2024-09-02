#include "util_headers.h"

#define SHM_MAX_LENGTH 50
// File: {20 chars}.txt | MD5: {32 chars} | PID: {5 chars}\0

// Devuelve el largo del string guardado
// Data siempre tendra espacio
int read_shm(char* data, char* buff, int* flag);

int open_shm(char* shm_name, int flag);
void * map_shm(int fd, size_t size, int flag);

int main(int argc, char** argv){
    int flag = 0, space_counter = 1, char_counter = 0;
    char data[MEMORY_CHUNCK];
    char * shm_name;

    if (argc < 2){
        shm_name = malloc(SHM_MAX_LENGTH);
        read(STDIN_FILENO, shm_name, SHM_MAX_LENGTH);
    } else{
        shm_name = argv[1];
    }

    printf("Shared memory name: %s\n", shm_name);

    // SHARED MEMORY TO READ DATA
    int fd = open_shm(shm_name, O_RDONLY);

    // READ-LENGTH es el maximo tamaño que puede tener en primera instancia lo que debo imprimir
    char* buffer = map_shm(fd, MEMORY_CHUNCK * sizeof(char), PROT_READ);
    //----------------------------


    // SEMAPHORE TO KNOW WHEN READ
    int sem_fd = open_shm(SHM_SEM_NAME, O_RDWR);

    sem_t* sem = map_shm(sem_fd, sizeof(sem_t), PROT_READ | PROT_WRITE);
    //----------------------------

    while (flag == 0)
    {
        sem_wait(sem);

        char_counter += read_shm(data, buffer, &flag);  // esto lo puedo interrumpir

        buffer = mmap(NULL, char_counter + MEMORY_CHUNCK, PROT_READ, MAP_SHARED, fd, 0);    // mapeo devuelta con el posible espacio que voy a tener

        buffer += char_counter; // desfaso segun los caracteres leidos

        if (flag == 0)
        {
            space_counter++;
            printf("%s\n", data);
        }   
    }

    if (argc < 2)       // quiere decir que hice un malloc
    {
        free(shm_name);
    }
    
    
    munmap(sem, sizeof(sem_t));             // Desmapeamos el espacio de memoria del programa
    shm_unlink(SHM_SEM_NAME);               // desvinculamos el nombre de la shared memory
    close(sem_fd);                          // cerramos el file descriptor

    munmap(buffer, (sizeof(char) * space_counter * MEMORY_CHUNCK) + 1);
    shm_unlink(shm_name);
    close(fd);

    return 0;
}

int read_shm(char* data, char* buff, int* flag){
    int i;

    for (i = 0; i < MEMORY_CHUNCK - 1 && buff[i] != 0 && buff[i] != ASCII_EOF ; i++)
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


int open_shm(char* shm_name, int flag){

    int fd = shm_open(shm_name, flag, 0);

    if (fd == -1){
        printf("Cannot open %s to read.\n", shm_name);
        exit(-1);
    }

    return fd;
}

void * map_shm(int fd, size_t size, int flag){
    void* buffer = mmap(NULL, size, flag, MAP_SHARED, fd, 0);

    if (buffer == MAP_FAILED){
        printf("Cannot map shared memory.\n");
        exit(-1);
    }

    return buffer;
}