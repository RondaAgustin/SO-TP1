// Esto es un programa provisorio que simula el comportamiento de un esclavo. Recibe tareas por stdin y las imprime por stdout.

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    char buffer[256];
    ssize_t bytes_read;
    size_t total_bytes = 0;

    while (1) {
        char byte;
        bytes_read = read(STDIN_FILENO, &byte, 1);
        if (bytes_read < 0) {
            perror("read");
            return 1;
        } else if (bytes_read == 0) {
            // EOF
            break;
        }

        buffer[total_bytes++] = byte;

        // Verificar si se ha recibido el terminador de cadena '\0'
        if (byte == '\0') {
            sleep(3);
            char output[60];
            sprintf(output, "%s:b23cfc5374362db7c6cbf6efba710f1b", buffer);
            write(STDOUT_FILENO, output, strlen(output));
            total_bytes = 0; // Reiniciar el contador de bytes para la próxima lectura
        }

        // Evitar desbordamiento del buffer
        if (total_bytes >= sizeof(buffer)) {
            fprintf(stderr, "Buffer overflow\n");
            return 1;
        }
    }

    return 0;
}