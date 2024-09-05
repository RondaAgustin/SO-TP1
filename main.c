#include "utils.h"
#include <string.h>

#define SLAVES_QTY 5
#define TASKS_QTY 2

#define SHM_NAME "md5_shm\0"          // este define queda aca

void write_to_result_file(char* buffer);
int share_to_view_process(char *buffer, const char *data);

typedef struct CompletionStatus {
    int tasks_sent;
    int tasks_completed;
    int tasks_completed_by_slave[SLAVES_QTY];
} CompletionStatus;

typedef struct ComunicationPipes {
    int request[SLAVES_QTY][2];
    int response[SLAVES_QTY][2];
} ComunicationPipes;

int main(int argc, char *argv[]) {

    pid_t pids[SLAVES_QTY];
    int filesQty = argc - 1;

    CompletionStatus* completion_status = malloc(sizeof(CompletionStatus));
    ComunicationPipes* comunication_pipes = malloc(sizeof(ComunicationPipes));

    // Create shm to share semaphores
    int sem_fd = open_shm(SHM_SEM_NAME, 
                        O_CREAT | O_RDWR,
                        0666
                        );

    create_shm_space(SHM_NAME, sem_fd, sizeof(sem_t));

    sem_t *shm_sem = map_shm(sem_fd, sizeof(sem_t), PROT_READ | PROT_WRITE);
        
    if (sem_init(shm_sem, 1, 0) == -1)      // creamos un semaforo que se compartira y lo mapeamos a la shm
    {
        printf("Cannot create semaphore.\n");
        return -1;
    }

    //-------------------------------------------------------------------------------------------------


    // Create and print shared memory to connect view proces
    char* shm_buffer;

    int total_size = MEMORY_CHUNK * (filesQty) + 1;     // el + 1 hace referencia al ASCII_EOF
    
    int fd = open_shm(SHM_NAME, 
                    O_CREAT | O_RDWR,   // creamos si no existe y se abre para lectura y escritura
                    0666                // indica permisos de lectura y escritura para todos los usuarios
    );

    create_shm_space(SHM_NAME, fd, sizeof(char) * total_size);
    
    shm_buffer = map_shm(fd, sizeof(char) * total_size, PROT_READ | PROT_WRITE);  

    shm_buffer[total_size - 1] = ASCII_EOF;

    printf("%s",SHM_NAME);             // Compartimos el nombre de la shared memory ya creada por salida estandar
    fflush(stdout);                     // Fuerzo a imprimir
    sleep(2);
    //-------------------------------------------------------------------------------------------------

    // Iniciamos los procesos esclavos
    for(int i = 0; i < SLAVES_QTY; i++) {
        pipe(comunication_pipes->request[i]);                     // Creo un pipe para enviar las peticiones de hasheo al esclavo.
        pipe(comunication_pipes->response[i]);                    // Creo un pipe para recibir el MD5 del archivo que envié como request.
        pids[i] = fork();                           // Forkeo el proceso para dividir la ejecución en dos y almaceno el PID (en el proceso padre) en el array pids.

        if(pids[i] == -1) {
            perror("Error al crear el pipe del proceso esclavo");
            exit(1);
        }

        if(pids[i] == 0) {                          // Estamos en el proceso hijo.
            // TODO: cerrar pipes de procesos creados con anterioridad, desde 0 hasta i - 1. Chequear con lsof y CTRL-Z.

            // Configuramos los pipes.
            close(comunication_pipes->request[i][1]);             // Cierro el extremo de escritura del request pipe (no lo voy a usar).
            close(comunication_pipes->response[i][0]);            // Cierro el extremo de lectura del response pipe (no lo voy a usar).
            dup2(comunication_pipes->request[i][0], STDIN_FILENO); // Redirijo la entrada estándar al extremo de lectura del request pipe.
            dup2(comunication_pipes->response[i][1], STDOUT_FILENO); // Redirijo la salida estándar al extremo de escritura del response pipe.
            close(comunication_pipes->request[i][0]);              // Cierro el extremo de lectura del request pipe (ya lo redirigí).
            close(comunication_pipes->response[i][1]);             // Cierro el extremo de escritura del response pipe (ya lo redirigí).
            int res = execve("./slave", NULL, NULL);           // Ejecuto el programa esclavo.
            if(res == -1) {
                char errorMsg[49];
                sprintf(errorMsg, "Error al ejecutar el programa esclavo con PID %d\n", getpid());
                perror(errorMsg);
                exit(1);
            }
        } else {                                    // Estamos en el proceso padre.
            // Configuramos los pipes.
            close(comunication_pipes->request[i][0]);             // Cierro el extremo de lectura del request pipe (no lo voy a usar).
            close(comunication_pipes->response[i][1]);            // Cierro el extremo de escritura del response pipe (no lo voy a usar).
        }
    }


    // Ya con los esclavos creados, les enviamos una cantidad TASKS_QTY de tareas iniciales a cada uno.
    int continue_sending = 1;
    for (int i = 0; i < SLAVES_QTY && continue_sending; i++) {
        for(int j = 0; j < TASKS_QTY; j++) {
            if(completion_status->tasks_sent >= filesQty) {
                continue_sending = 0;
                break;
            }
            char* task = argv[completion_status->tasks_sent + 1];
            write(comunication_pipes->request[i][1], task, strlen(task) + 1);
            completion_status->tasks_sent++;
        }
    }

    // Una vez que los esclavos tienen tareas, el proceso padre procede a escuchar cuando alguno de los pipes tiene información disponible.
    // para leer. Cuando lea un MD5, lo agrega al archivo resultado.txt y lo manda al proceso vista (en caso de existir).

    fd_set response_pipes_fds; // Estos son los file descriptors que vamos a monitorear para saber si hay información disponible para leer.

    while(completion_status->tasks_completed < filesQty) {
        FD_ZERO(&response_pipes_fds);                       // Los inicializamos todos en 0 en cada iteración del while, esto es necesario dado que select() modifica el set que le pasamos para indicar qué file descriptors tienen información disponible.
        int max_fd = -1;                                    // Usamos esta variable para guardar el file descriptor más grande que vamos a monitorear, dado que select() necesita que le pasemos esto (+1) por parámetro.

        for (int i = 0; i < SLAVES_QTY; i++) {
            int response_pipe_read_fd = comunication_pipes->response[i][0];
            FD_SET(response_pipe_read_fd, &response_pipes_fds);
            if (response_pipe_read_fd > max_fd) {
                max_fd = response_pipe_read_fd;
            }
        }

        // Esperamos a que haya información disponible para leer en alguno de los file descriptors que monitoreamos.
        int data_available = select(max_fd + 1, &response_pipes_fds, NULL, NULL, NULL);

        if(data_available == -1) {
            perror("Error al ejecutar la función select()");
            exit(1);
        }

        // Ahora verificamos en cuál de los pipes hay información disponible para leer.
        for (int i = 0; i < SLAVES_QTY; i++) {
            // Preguntamos si en el pipe de índice i hay información disponible para leer.
            int read_fd = comunication_pipes->response[i][0];
            if (FD_ISSET(read_fd, &response_pipes_fds)) {
                completion_status->tasks_completed++;
                completion_status->tasks_completed_by_slave[i]++;

                // Hay un MD5 disponible en el pipe, lo leemos.
                char md5_response_buffer[256];
                read_until_end_of_string(read_fd, md5_response_buffer, 256);
                
                // El resultado que obtenemos será de la forma nombre_archivo:md5, entonces spliteamos el string.
                char* filename = strtok(md5_response_buffer, ":");
                char* md5 = strtok(NULL, ":");

                // Le damos formato al output que vamos a generar
                char buffer[MEMORY_CHUNK];
                sprintf(buffer, "File: %s | MD5: %s | PID: %d", filename, md5, pids[i]);

                // Agregamos el resultado al archivo resultado.txt.
                write_to_result_file(buffer);
                sem_post(shm_sem);

                // Compartimos el resultado con el proceso vista.
                shm_buffer += share_to_view_process(shm_buffer, buffer);

                // Verificamos si los esclavos completaron las tareas iniciales y luego si hay más tareas para enviar.
                if(completion_status->tasks_completed_by_slave[i] >= TASKS_QTY) {
                    if(completion_status->tasks_sent < filesQty) {
                        char* task = argv[completion_status->tasks_sent + 1];
                        write(comunication_pipes->request[i][1], task, strlen(task) + 1);
                        completion_status->tasks_sent++;
                    } else {
                        // No hay más tareas para enviar, cerramos el pipe de escritura para que el esclavo termine.
                        close(comunication_pipes->request[i][1]);
                    }
                }
            }
        }
    }

    *shm_buffer = ASCII_EOF;        // ponemos en el EOF para que el programa vista sepa que ya esta
    sem_post(shm_sem);

    munmap(shm_buffer, (sizeof(char) * total_size) + 1);
    close(fd);


    munmap(shm_sem, sizeof(sem_t));             // Desmapeamos el espacio de memoria del programa
    close(sem_fd);                          // cerramos el file descriptor

    free(completion_status);
    free(comunication_pipes);

    return 0;

}

void write_to_result_file(char* buffer) {
    FILE* result_file = fopen("resultado.txt", "a");
    if(result_file == NULL) {
        perror("Error al abrir el archivo resultado.txt");
        exit(1);
    }
    fprintf(result_file, "MD5: %s\n", buffer);
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