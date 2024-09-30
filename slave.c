// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    char buffer[MAX_STRING_LENGTH];
    ssize_t bytes_read;
    size_t total_bytes = 0;

    while (1) {
        int pipe_fds[2];
        char byte;

        bytes_read = read(STDIN_FILENO, &byte, 1);

        if (bytes_read == 0) {
            // EOF
            break;
        }

        // Evitar desbordamiento del buffer
        if (total_bytes >= sizeof(buffer)) {
            fprintf(stderr, "Buffer overflow\n");
            return 1;
        }

        buffer[total_bytes++] = byte;

        // Verificar si se ha recibido el terminador de cadena '\0'
        if (byte == '\0') {

            pipe(pipe_fds);
            int md5_pid;
            if((md5_pid = fork()) == 0) {
                close(pipe_fds[0]);
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[1]);
                char *args[] = {"/usr/bin/md5sum", buffer, NULL};
                execve(args[0], args, NULL);
                return -1;
            } else {
                close(pipe_fds[1]);
                char md5[MD5_LENGTH + 1]; // 32 caracteres + 1 para el terminador de cadena '\0'
                read(pipe_fds[0], md5, MD5_LENGTH);
                md5[MD5_LENGTH] = '\0'; // Asegurarse de que la cadena esté terminada en nulo
                char output[MAX_STRING_LENGTH + MD5_LENGTH + 1];
                sprintf(output, "%s:%s", buffer, md5);
                write(STDOUT_FILENO, output, strlen(output) + 1);
                total_bytes = 0; // Reiniciar el contador de bytes para la próxima lectura
                close(pipe_fds[0]);
                waitpid(md5_pid, NULL, 0);
            }
        }
    }
    return 0;
}