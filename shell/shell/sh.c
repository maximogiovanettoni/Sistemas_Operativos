#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

char prompt[PRMTLEN] = { 0 };
static void *alt_stack = NULL;

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

void
sigchild_handler(int signo)
{
	int status;
	pid_t pid;
	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		printf_debug("==> terminado: PID=%d\n", pid);
	}
}
void
set_signal_handler()
{
	alt_stack = malloc(STACK_SIZE);
	if (alt_stack == NULL) {
		printf_debug(stderr, "malloc del stack falló.");
		exit(EXIT_FAILURE);
	}
	stack_t ss = { .ss_sp = alt_stack, .ss_size = STACK_SIZE, .ss_flags = 0 };
	if (sigaltstack(&ss, NULL) == -1) {
		perror("sigaltstack");
		free(alt_stack);
		exit(EXIT_FAILURE);
	}
	struct sigaction sa;
	sa.sa_handler = sigchild_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_ONSTACK | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		free(alt_stack);
		exit(EXIT_FAILURE);
	}
}

void
liberar_alt_stack()
{
	if (alt_stack != NULL) {
		free(alt_stack);
		alt_stack = NULL;
	}
}

int
main(void)
{
	set_signal_handler();
	init_shell();
	run_shell();
	liberar_alt_stack();
	return 0;
}
