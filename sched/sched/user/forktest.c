#include <inc/lib.h>

const int TIME_LAPSE = 1000000000;
const int N = 1000000000;

void
umain(int argc, char **argv)
{
	envid_t env;
	cprintf("Forking test with priority scheduler\n");
	int parent_priority = thisenv->priority;
	cprintf("Parent process priority at start: %d\n", parent_priority);
	int f = fork();
	if (f < 0) {
		cprintf("fork failed\n");
	} else if (f == 0) {
		int child_priority =
		        thisenv->priority;  // Obtener la prioridad del proceso hijo
		cprintf("Child process priority at start: %d\n", child_priority);
		for (int i = 0; i < N; i++) {
			cprintf("Child process running with priority: %d\n",
			        thisenv->priority);
			for (int j = 0; i < TIME_LAPSE; i++) {
				continue;
			}
		}
	}
	for (int i = 0; i < N; i++) {
		cprintf("Parent process running with priority: %d\n",
		        thisenv->priority);
		for (int j = 0; i < TIME_LAPSE; i++) {
			continue;
		}
	}
}