// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "utils.h"
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


#define SLAVES_QTY 5
#define TASKS_QTY 2

#define SHM_NAME "md5_shm\0"          // este define queda aca


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
    signal(SIGPIPE, SIG_IGN);       // decidimos ignorar la señal SIGPIPE para que no haya leaks al correr el main | view

    pid_t pids[SLAVES_QTY];
    int filesQty = argc - 1;

    char * slave_argv[] = {SLAVE_FILE_NAME, NULL};

    CompletionStatus* completion_status = calloc(1, sizeof(CompletionStatus));
    if (completion_status == NULL){
        perror("Cannot allocate completion_status\n");
        exit(-1);
    }
    
    ComunicationPipes* comunication_pipes = calloc(1, sizeof(ComunicationPipes));
    if (comunication_pipes == NULL){
        free(completion_status);
        perror("Cannot allocate comunication_pipes\n");
        exit(-1);
    }
    

    // Create shm to share semaphores
    sem_t * shm_sem;

    int sem_fd = open_shm(SHM_SEM_NAME, 
                        O_CREAT | O_RDWR,
                        0666
                    );

    create_shm_space(SHM_NAME, sem_fd, sizeof(sem_t));

    shm_sem = map_shm(sem_fd, sizeof(sem_t), PROT_READ | PROT_WRITE);
        
    if (sem_init(shm_sem, 1, 0) == -1)      // creamos un semaforo que se compartira y lo mapeamos a la shm
    {
        printf("Cannot create semaphore.\n");
        return -1;
    }

    //-------------------------------------------------------------------------------------------------


    // Create and print shared memory to connect view proces
    char *shm_buffer, *shm_buffer_begin;

    int total_size = MEMORY_CHUNK * (filesQty) + 1;     // el + 1 hace referencia al ASCII_EOF
    
    int fd = open_shm(SHM_NAME, 
                    O_CREAT | O_RDWR,   // creamos si no existe y se abre para lectura y escritura
                    0666                // indica permisos de lectura y escritura para todos los usuarios
    );

    create_shm_space(SHM_NAME, fd, sizeof(char) * total_size);
    
    shm_buffer_begin = map_shm(fd, sizeof(char) * total_size, PROT_READ | PROT_WRITE);  

    shm_buffer_begin[total_size - 1] = ASCII_EOF;

    shm_buffer = shm_buffer_begin;

    printf("%s\n",SHM_NAME);             // Compartimos el nombre de la shared memory ya creada por salida estandar
    fflush(stdout);

    sleep(CONNECT_TIME);
    
    printf("\n");
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
            // TODO: Chequear que los pipes esten bien cerrados con lsof y CTRL-Z (probar que si se quedan abiertos comentando esto y que se cierran al descomentar)
            for(int k = 0; k < i; k++) {
                close(comunication_pipes->response[k][0]);
                close(comunication_pipes->request[k][1]);
            }

            // Configuramos los pipes.
            close(comunication_pipes->request[i][1]);             // Cierro el extremo de escritura del request pipe (no lo voy a usar).
            close(comunication_pipes->response[i][0]);            // Cierro el extremo de lectura del response pipe (no lo voy a usar).
            dup2(comunication_pipes->request[i][0], STDIN_FILENO); // Redirijo la entrada estándar al extremo de lectura del request pipe.
            dup2(comunication_pipes->response[i][1], STDOUT_FILENO); // Redirijo la salida estándar al extremo de escritura del response pipe.
            close(comunication_pipes->request[i][0]);              // Cierro el extremo de lectura del request pipe (ya lo redirigí).
            close(comunication_pipes->response[i][1]);             // Cierro el extremo de escritura del response pipe (ya lo redirigí).
            int res = execve(SLAVE_FILE_NAME, slave_argv, NULL);           // Ejecuto el programa esclavo.
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
                char md5_response_buffer[MAX_STRING_LENGTH];
                read_until_end_of_string(read_fd, md5_response_buffer, MAX_STRING_LENGTH);
                
                // El resultado que obtenemos será de la forma nombre_archivo:md5, entonces spliteamos el string.
                char* filename = strtok(md5_response_buffer, ":");
                char* md5 = strtok(NULL, ":");

                if (filename != NULL && md5 != NULL){
                    // Le damos formato al output que vamos a generar
                    char buffer[MEMORY_CHUNK];
                    sprintf(buffer, "File: %s | MD5: %s | PID: %d", filename, md5, pids[i]);

                    // Agregamos el resultado al archivo resultado.txt.
                    write_to_result_file(buffer);

                    // Compartimos el resultado con el proceso vista.
                    shm_buffer += share_to_view_process(shm_buffer, buffer);
                    sem_post(shm_sem);

                    // Verificamos si los esclavos completaron las tareas iniciales y luego si hay más tareas para enviar.
                    if(completion_status->tasks_completed_by_slave[i] >= TASKS_QTY) {
                        if(completion_status->tasks_sent < filesQty) {
                            char* task = argv[completion_status->tasks_sent + 1];
                            write(comunication_pipes->request[i][1], task, strlen(task) + 1);
                            completion_status->tasks_sent++;
                        }
                    }
                }
            }
        }
    }

    // Cerramos los pipes que quedaron abiertos.
    for(int i = 0; i < SLAVES_QTY; i++) {
        close(comunication_pipes->response[i][0]);
        close(comunication_pipes->request[i][1]);
    }

    *shm_buffer = ASCII_EOF;        // ponemos en el EOF para que el programa vista sepa que ya esta
    sem_post(shm_sem);



    if (munmap(shm_sem, sizeof(sem_t)) == -1) {
        perror("Error al desmapear shm_sem\n");
    }
    close(sem_fd);                          // cerramos el file descriptor
    if (shm_unlink(SHM_SEM_NAME) == -1){
        perror("Error al eliminar shared memory for sem.\n");
    }

    if (munmap(shm_buffer_begin, sizeof(char) * total_size) == -1) {    // Desmapeamos el espacio de memoria del programa
        perror("Error al desmapear shm_buffer\n");
    }
    close(fd);
    if (shm_unlink(SHM_NAME) == -1){
        perror("Error al eliminar shared memory.\n");
    }


    free(completion_status);
    free(comunication_pipes);

    for (int i = 0; i < SLAVES_QTY; i++){
        waitpid(pids[i], NULL, 0);
    }

    return 0;
}