#ifndef NARGS
#define NARGS 4
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {


    char *xargs[NARGS + 2]; 
    xargs[0] = argv[1];  
    xargs[NARGS + 1] = NULL; 

    char *linea = NULL;
    size_t len = 0;
    ssize_t leido;
    int cantidad = 0;

    while ((leido = getline(&linea, &len, stdin)) != -1) {
        // Leer los argumentos línea a línea, nunca separados por espacios 
		//(se recomienda usar la función getlinea(3)). También, es necesario eliminar
		// el caracter '\n' para obtener el nombre del archivo.
        if (linea[leido - 1] == '\n') {
            linea[leido - 1] = '\0';
        }

        // Alamcenar cuanto mucho NARGS argumentos en un buffer. 
		//Es decir, asumir que el stream de datos podría ser “infinito”.
        xargs[cantidad + 1] = strdup(linea);
        cantidad++;

        // Siempre se pasan NARGS argumentos al comando ejecutado 
        if (cantidad == NARGS) {
            if (fork() == 0) {
                execvp(xargs[0], xargs);
                perror("fallo execvp");
                exit(EXIT_FAILURE);
            } else {
                wait(NULL);  // Se debe esperar siempre a que termine la ejecución 
							//  del comando actual.
            }
            cantidad = 0; 
        }
    }

    //(excepto en su última ejecución, que pueden ser menos).
    if (cantidad > 0) {
        xargs[cantidad + 1] = NULL;  
        if (fork() == 0) {
            execvp(xargs[0], xargs);
            perror("fallo execvp");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }
    }
    return 0;
}
