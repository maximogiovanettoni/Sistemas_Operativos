#include "exec.h"
#include <stdlib.h>
#include "defs.h"
#include "printstatus.h"
// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//

static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	// Recorro cada variable de entorno
	// Ej: KEY=VALUE, KEY2=VALUE2 ... KEYn=VALUEn
	for (size_t i = 0; i < eargc; ++i) {
		char *env_var = eargv[i];  // KEY=VALUE
		int equal_index = block_contains(
		        env_var,
		        '=');  // Busco el indice del igual, para separar, clave y valor
		if (equal_index < 0)
			continue;  // Si no hay igual, sigo con la siguiente variable de entorno
		else {
			char *key = (char *) malloc(
			        sizeof(char) *
			        (equal_index + 1));  // Creo espacio para la clave
			char *value = (char *) malloc(
			        sizeof(char) *
			        (strlen(env_var) -
			         equal_index));  // Creo espacio para el valor
			if (key == NULL) {
				// Error
				fprintf(stderr,
				        "Error allocating memory for key %zu\n",
				        i);
				continue;
			}
			if (value == NULL) {
				// Error
				free(key);
				fprintf(stderr, "Error allocating memory for value %zu\n", i);
				continue;
			}

			get_environ_key(env_var,
			                key);  // Seteo la clave a la variable key
			get_environ_value(
			        env_var,
			        value,
			        equal_index);  // Seteo la clave a la variable value

			if (setenv(key, value, 1) !=
			    0)  // Seteo la variable de entorno
			{
				// Error
				fprintf(stderr, "Error setting environment variable %s\n", key);
				continue;
			}
			free(key);    // Libero memoria
			free(value);  // Libero memoria
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags & O_CREAT) {
		fd = open(file, flags, S_IWUSR | S_IRUSR);
	} else {
		fd = open(file, flags);
	}

	if (fd < 0) {
		perror("Error abriendo el archivo.");
		return -1;
	}

	return fd;
}

static void
caso_stdin(char *in_file)
{
	if (strlen(in_file) > 0) {
		int in_fd = open_redir_fd(in_file, O_RDONLY | O_CLOEXEC);

		if (in_fd < 0) {
			perror("Error abriendo el archivo de entrada.");
			exit(EXIT_FAILURE);
		}

		if (dup2(in_fd, STDIN_FILENO) < 0) {
			perror("Error redireccionando la entrada.");
			close(in_fd);
			exit(EXIT_FAILURE);
		}
		close(in_fd);
	}
}

static void
caso_stdout(char *out_file)
{
	if (strlen(out_file) > 0) {
		int out_fd =
		        open_redir_fd(out_file,
		                      O_WRONLY | O_CREAT | O_CLOEXEC | O_TRUNC);

		if (dup2(out_fd, STDOUT_FILENO) < 0) {
			perror("Error redirecting the exit.");
			exit(EXIT_FAILURE);
		}
		close(out_fd);
	}
}

static void
caso_stderr(char *err_file)
{
	if (strlen(err_file) > 0) {
		if (strcmp(err_file, "&1") == 0) {
			if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
				perror("Error redirecting the error.");
				exit(EXIT_FAILURE);
			}
		} else {
			int err_fd = open_redir_fd(err_file,
			                           O_WRONLY | O_CREAT |
			                                   O_CLOEXEC | O_TRUNC);
			if (dup2(err_fd, STDERR_FILENO) < 0) {
				perror("Error redirecting the errors.");
				exit(EXIT_FAILURE);
			}
			close(err_fd);
		}
	}
}

static void
caso_pipe(struct pipecmd *p)
{
	int pipefds[2];
	pid_t left_pid, right_pid;

	if (pipe(pipefds) < 0) {
		perror("Error creating pipe.");
		exit(EXIT_FAILURE);
	}

	left_pid = fork();
	if (left_pid < 0) {
		perror("Error forking left process.");
		close(pipefds[0]);
		close(pipefds[1]);
		exit(EXIT_FAILURE);
	}

	if (left_pid == 0) {
		free_command(p->rightcmd);
		close(pipefds[0]);  // Cerrar lectura en el hijo 1

		if (dup2(pipefds[1], STDOUT_FILENO) < 0) {
			perror("Error duplicating the descriptor.");
			exit(EXIT_FAILURE);
		}
		close(pipefds[1]);
		exec_cmd(p->leftcmd);  // Ejecutar el comando izquierdo
		perror("Error executing left command.");
		exit(EXIT_FAILURE);
	}

	right_pid = fork();
	if (right_pid < 0) {
		perror("Error forking left process.");
		close(pipefds[0]);
		close(pipefds[1]);
		exit(EXIT_FAILURE);
	}

	if (right_pid == 0) {
		free_command(p->leftcmd);
		close(pipefds[1]);  // Cerrar escritura en el hijo 2
		if (dup2(pipefds[0], STDIN_FILENO < 0)) {
			perror("Error creating a new process.");
			exit(EXIT_FAILURE);
		}
		close(pipefds[0]);
		exec_cmd(p->rightcmd);  // Ejecutar el comando derecho
		perror("Error executing left command.");
		exit(EXIT_FAILURE);
	}

	// Cerrar ambos extremos del pipe en el padre
	close(pipefds[0]);
	close(pipefds[1]);

	// Esperar a ambos procesos hijos
	waitpid(left_pid, &status, 0);
	waitpid(right_pid, &status, 0);
	exit(EXIT_SUCCESS);
}
// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		//
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		if (e->argv[0] == NULL) {
			fprintf(stderr, "No command found.\n");
			exit(EXIT_FAILURE);
		}
		execvp(e->argv[0], e->argv);
		fprintf(stderr, "execvp failed.");
		exit(EXIT_FAILURE);

	case BACK: {
		// runs a command in background
		//
		struct backcmd *b = (struct backcmd *) cmd;
		exec_cmd(b->c);

		break;
	}
	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		r = (struct execcmd *) cmd;
		caso_stdin(r->in_file);
		caso_stdout(r->out_file);
		caso_stderr(r->err_file);
		r->type = EXEC;
		exec_cmd((struct cmd *) r);
		break;
	}
	case PIPE: {
		// pipes two commands
		//
		p = (struct pipecmd *) cmd;
		caso_pipe(p);
		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);
		break;
	}
	}
	exit(0);
}
