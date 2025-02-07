#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void calcular_primo(int pipe_izq[2])
{
	close(pipe_izq[1]);
	// Todos los procesos sucesivos aplican el pseudo-código mostrado anteriormente,
	// con la salvedad de que son responsables de crear a su “hermano” derecho,
	//  y la tubería (pipe) que los comunica.
	int p;
	if (read(pipe_izq[0], &p, sizeof(int)) > 0) {
		printf("primo %d\n", p);  // asumiendo que es primo

		int pipe_der[2];
		pipe(pipe_der);

		if (fork() == 0) {
			close(pipe_izq[0]);
			calcular_primo(pipe_der);
		} else {
			close(pipe_der[0]);
			int n;
			while (read(pipe_izq[0], &n, sizeof(int)) >
			       0) {  // mientras <pipe izquierdo no cerrado>:
				     //<leer siguiente valor de pipe izquierdo>
				if (n % p != 0) {
					write(pipe_der[1], &n, sizeof(int));  // escribir <n> en el pipe derecho
				}
			}
			close(pipe_der[1]);
			close(pipe_izq[0]);
			wait(NULL);
		}
	}
	exit(0);
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Uso: %s <n>\n", argv[0]);
		return 1;
	}

	int n = atoi(argv[1]);
	if (n < 2) {
		fprintf(stderr, "El número debe ser mayor o igual a 2.\n");
		return 1;
	}
	// El primer proceso cree un proceso derecho, con el que se comunica mediante un pipe.
	int fds[2];
	pipe(fds);

	if (fork() == 0) {
		calcular_primo(fds);
	} else {
		// Ese primer proceso, escribe en el pipe la secuencia de números de 2 a n,
		// para a continuación cerrar el pipe y esperar la finalización del proceso derecho.
		close(fds[0]);
		for (int i = 2; i <= n; i++) {
			write(fds[1], &i, sizeof(int));
		}
		close(fds[1]);
		wait(NULL);
	}

	return 0;
}
