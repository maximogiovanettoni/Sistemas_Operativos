#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

const int BOOST_INTERVAL = 20;
const int PRIORITY_TOLERANCE = 2;

void show_stats();
void sched_halt(void);

void
envs_boost_priority()
{
	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			envs[i].priority = DEFAULT_PRIORITY;
		}
	}
	env_statistics.priority_boosts++;
}

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin
	env_statistics.sched_calls++;
	struct Env *next_env = curenv;
	int primer_indice = next_env ? ENVX(next_env->env_id) + 1 : 0;

	for (int i = 0; i < NENV; i++) {
		int indice_actual = (primer_indice + i) % NENV;
		next_env = &envs[indice_actual];
		if (next_env->env_status == ENV_RUNNABLE) {
			env_statistics.env_rans++;
			env_run(next_env);
		}
	}
	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_statistics.env_rans++;
		env_run(curenv);
	}

	sched_halt();
#endif
	// Si no hay mas proceso para correr se lo llama a sched_halt


#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities

	// ROUND ROBIN CON PRIORIDADES

	// Tener en cuenta que prioridad 5 es la mas alta y 0 la mas baja
	// Si llegan 2 procesos uno con prioridad 5 y otro con prioridad 0, se corre el de prioridad 5
	env_statistics.sched_calls++;
	struct Env *next_env = curenv;
	int primer_indice = next_env ? ENVX(next_env->env_id) + 1 : 0;


	if (env_statistics.env_rans % BOOST_INTERVAL == 0) {
		envs_boost_priority();
	}

	int max_priority = -1;
	struct Env *max_env = NULL;

	for (int i = 0; i < NENV; i++) {
		int indice_actual = (primer_indice + i) % NENV;
		next_env = &envs[indice_actual];
		if (next_env->env_status == ENV_RUNNABLE &&
		    next_env->priority >= max_priority) {
			if (next_env->priority == max_priority &&
			    next_env->env_runs < max_env->env_runs) {
				max_env = next_env;
				max_priority = next_env->priority;
			} else {
				max_env = next_env;
				max_priority = next_env->priority;
			}
		}
	}

	if (max_env) {
		env_statistics.env_rans++;
		max_env->priority_counter++;
		if (max_env->priority_counter == PRIORITY_TOLERANCE) {
			if (max_env->priority > MIN_PRIORITY) {
				max_env->priority--;
			}
			max_env->priority_counter = 0;
		}
		env_run(max_env);
	}

	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_statistics.env_rans++;
		env_run(curenv);
	}

	sched_halt();
#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		env_run(curenv);
	}

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	show_stats();

	int i;

	cprintf("Nothing to do! CPU %d halted.\n",
	        cpunum());  // Solo para debugging

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here
	///*  */show_stats();
	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}

void
show_stats()
{
	cprintf("\n");
	cprintf("----------------------Scheduler "
	        "statistics----------------------\n");
	cprintf("Number of scheduler calls: %d\n", env_statistics.sched_calls);
	cprintf("Number of priority boosts: %d\n", env_statistics.priority_boosts);
	cprintf("Number of environments executed: %d\n", env_statistics.env_rans);
	cprintf("\n");
}