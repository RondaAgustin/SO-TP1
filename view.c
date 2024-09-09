#include "utils.h"

#define SHM_MAX_LENGTH 50
// File: {20 chars}.txt | MD5: {32 chars} | PID: {5 chars}\0

int main(int argc, char** argv){
    int flag = 0, space_counter = 1, char_counter = 0;
    char data[MEMORY_CHUNK];
    char * shm_name;

    if (argc < 2){
        shm_name = malloc(SHM_MAX_LENGTH);
        read(STDIN_FILENO, shm_name, SHM_MAX_LENGTH);
    } else{
        shm_name = argv[1];
    }

    printf("Shm to read: %s\n", shm_name);
    // SHARED MEMORY TO READ DATA
    int fd = open_shm(shm_name, O_RDONLY, 0);

    // READ-LENGTH es el maximo tamaño que puede tener en primera instancia lo que debo imprimir
    char* buffer = map_shm(fd,  MEMORY_CHUNK * sizeof(char), PROT_READ);
    //----------------------------


    // SEMAPHORE TO KNOW WHEN READ
    int sem_fd = open_shm(SHM_SEM_NAME, O_RDWR, 0);

    sem_t* sem = map_shm(sem_fd, sizeof(sem_t), PROT_READ | PROT_WRITE);
    //----------------------------


    while (flag == 0)
    {
        sem_wait(sem);

        char_counter += read_shm(data, buffer, &flag);  // esto lo puedo interrumpir

        buffer = mmap(NULL, (char_counter + MEMORY_CHUNK) * sizeof(char), PROT_READ, MAP_SHARED, fd, 0);    // mapeo devuelta con el posible espacio que voy a tener

        if (flag == 0)
        {
            space_counter++;
            printf("%s\n", data);
            
            buffer += char_counter; // desfaso segun los caracteres leidos
            space_counter++;
        }   
    }


    if (argc < 2)       // quiere decir que hice un malloc
    {
        free(shm_name);
    }
    
    sem_destroy(sem);
    
    if (munmap(sem, sizeof(sem_t)) == -1){  // Desmapeamos el espacio de memoria del programa
        perror("Error al desmapear sem.\n");
    }

    close(sem_fd);                          // cerramos el file descriptor
    
    if (shm_unlink(SHM_SEM_NAME) == -1){
        perror("Error al eliminar shared memory for sem.\n");
    }


    if (munmap(buffer, (sizeof(char) *  char_counter + MEMORY_CHUNK)) == -1){
        perror("Error al desmapear buffer.\n");
    }
    
    close(fd);
    
    if (shm_unlink(shm_name) == -1){
        perror("Error al eliminar shared memory.\n");
    }

    return 0;
}