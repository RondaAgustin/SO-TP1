#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SLAVES_QTY 5
#define TASKS_QTY 2

#define ASCII_EOF 26

// TODO: eliminar esta función
void debug_print(char* message, char* color);
void write_to_result_file(char* buffer);

int main(int argc, char *argv[]) {

    pid_t pids[SLAVES_QTY];
    int request_pipes[SLAVES_QTY][2];
    int response_pipes[SLAVES_QTY][2];
    int tasks_sent = 0;

    char file_qty_buffer[30];
    sprintf(file_qty_buffer, "Se recibieron %d archivos.", argc - 1);
    debug_print(file_qty_buffer, NULL);

    // Iniciamos los procesos esclavos
    for(int i = 0; i < SLAVES_QTY; i++) {
        pipe(request_pipes[i]);                     // Creo un pipe para enviar las peticiones de hasheo al esclavo.
        pipe(response_pipes[i]);                    // Creo un pipe para recibir el MD5 del archivo que envié como request.
        pids[i] = fork();                           // Forkeo el proceso para dividir la ejecución en dos y almaceno el PID (en el proceso padre) en el array pids.

        if(pids[i] == -1) {
            perror("Error al crear el pipe del proceso esclavo");
            exit(1);
        }

        if(pids[i] == 0) {                          // Estamos en el proceso hijo.
            // Configuramos los pipes.
            close(request_pipes[i][1]);             // Cierro el extremo de escritura del request pipe (no lo voy a usar).
            close(response_pipes[i][0]);            // Cierro el extremo de lectura del response pipe (no lo voy a usar).
            dup2(request_pipes[i][0], STDIN_FILENO); // Redirijo la entrada estándar al extremo de lectura del request pipe.
            dup2(response_pipes[i][1], STDOUT_FILENO); // Redirijo la salida estándar al extremo de escritura del response pipe.
            close(request_pipes[i][0]);              // Cierro el extremo de lectura del request pipe (ya lo redirigí).
            close(response_pipes[i][1]);             // Cierro el extremo de escritura del response pipe (ya lo redirigí).
            int res = execve("./slave", NULL, NULL);           // Ejecuto el programa esclavo.
            if(res == -1) {
                char errorMsg[49];
                sprintf(errorMsg, "Error al ejecutar el programa esclavo con PID %d\n", getpid());
                perror(errorMsg);
                exit(1);
            }
        } else {                                    // Estamos en el proceso padre.
            // Configuramos los pipes.
            close(request_pipes[i][0]);             // Cierro el extremo de lectura del request pipe (no lo voy a usar).
            close(response_pipes[i][1]);            // Cierro el extremo de escritura del response pipe (no lo voy a usar).
        }
    }

    debug_print("Esclavos creados", NULL);

    // Ya con los esclavos creados, les enviamos una cantidad TASKS_QTY de tareas iniciales a cada uno.
    int continue_sending = 1;
    for (int i = 0; i < SLAVES_QTY && continue_sending; i++) {
        for(int j = 0; j < TASKS_QTY; j++) {
            if(tasks_sent + 1 >= argc) {
                continue_sending = 0;
                break;
            }
            char* task = argv[tasks_sent + 1];
            write(request_pipes[i][1], task, strlen(task) + 1);
            tasks_sent++;
        }
    }

    debug_print("Tareas enviadas", NULL);

    // Una vez que los esclavos tienen tareas, el proceso padre procede a escuchar cuando alguno de los pipes tiene información disponible.
    // para leer. Cuando lea un MD5, lo agrega al archivo resultado.txt y lo manda al proceso vista (en caso de existir).

    fd_set response_pipes_fds; // Estos son los file descriptors que vamos a monitorear para saber si hay información disponible para leer.

    while(tasks_sent < argc) {
        FD_ZERO(&response_pipes_fds);                       // Los inicializamos todos en 0 en cada iteración del while, esto es necesario dado que select() modifica el set que le pasamos para indicar qué file descriptors tienen información disponible.
        int max_fd = -1;                                    // Usamos esta variable para guardar el file descriptor más grande que vamos a monitorear, dado que select() necesita que le pasemos esto (+1) por parámetro.

        for (int i = 0; i < SLAVES_QTY; i++) {
            int response_pipe_read_fd = response_pipes[i][0];
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
            int read_fd = response_pipes[i][0];
            if (FD_ISSET(read_fd, &response_pipes_fds)) {

                // Hay un MD5 disponible en el pipe, lo leemos.
                char md5_response_buffer[60];
                read(read_fd, md5_response_buffer, sizeof(md5_response_buffer));
                
                // El resultado que obtenemos será de la forma nombre_archivo:md5, entonces spliteamos el string.
                char* filename = strtok(md5_response_buffer, ":");
                char* md5 = strtok(NULL, ":");

                // Le damos formato al output que vamos a generar
                char buffer[86];
                sprintf(buffer, "File: %s | MD5: %s | PID: %d", filename, md5, pids[i]);

                // Agregamos el resultado al archivo resultado.txt.
                write_to_result_file(buffer);

                // Compartimos el resultado con el proceso vista.
                // TODO: implementar share_to_view_process(buffer);
                // share_to_view_process(buffer);

                // Verificamos si hay más tareas para enviar.
                if(tasks_sent + 1 < argc) {
                    char* task = argv[tasks_sent + 1];
                    write(request_pipes[i][1], task, strlen(task) + 1);
                    tasks_sent++;
                } else {
                    // No hay más tareas para enviar, cerramos el pipe de escritura para que el esclavo termine.
                    close(request_pipes[i][1]);
                }
            }
        }
    }

    return 0;

}

void write_to_result_file(char* buffer) {
    debug_print(buffer, "magenta");
    FILE* result_file = fopen("resultado.txt", "a");
    if(result_file == NULL) {
        perror("Error al abrir el archivo resultado.txt");
        exit(1);
    }
    fprintf(result_file, "MD5: %s\n", buffer);
    fclose(result_file);
}

// TODO: eliminar esta función
void debug_print(char* message, char* color) {

    if (color == NULL)
    {
        color = "cyan";
    }
    

    char* color_code;

    // Definir el código de color basado en el nombre del color
    if (strcmp(color, "red") == 0) {
        color_code = "\033[1;31m";
    } else if (strcmp(color, "green") == 0) {
        color_code = "\033[1;32m";
    } else if (strcmp(color, "yellow") == 0) {
        color_code = "\033[1;33m";
    } else if (strcmp(color, "blue") == 0) {
        color_code = "\033[1;34m";
    } else if (strcmp(color, "magenta") == 0) {
        color_code = "\033[1;35m";
    } else if (strcmp(color, "cyan") == 0) {
        color_code = "\033[1;36m";
    } else if (strcmp(color, "white") == 0) {
        color_code = "\033[1;37m";
    } else {
        // Color por defecto (sin color)
        color_code = "\033[0m";
    }

    // Imprimir el mensaje con el color especificado
    printf("%s%s\033[0m\n", color_code, message);
}