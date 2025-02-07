# Scheduler con prioridades

Para esta implementación decidimos inspirarnos en el mecanismo de Round Robin, recorrer el arreglo <i>(nuestro PCB)</i> hasta encontrar el primero en estado `ENV_RUNNABLE`, pero pero agregando una condición adicional clave. Agregamos una serie de reglas para que la función sched_yield elija el próximo proceso a ejecutar en base a un sistema de prioridades numérico. En principio, nuestro scheduler basa su sistema de prioridades en un rango numérico que va del 5, al que definimos `DEFAULT_PRIORITY` al ser el valor inicial de prioridad al que un proceso es asignado (el más alto), al 0, al que definimos como `MIN_PRIORITY` (este nombramiento habla por sí solo). Esta prioridad es asignada al momento de su alocación y modificada como un miembro del struct Env.

En fin, el mecanismo de búsqueda es idéntico a RR, se recorre el arreglo envs desde la posición siguiente al del curenv (current environment) de manera circular (en nuestra implementación nos ayudamos del operador módulo para esto); se detiene en los procesos cuyo estado sea igual a ENV_RUNNABLE y determina si la prioridad del proceso actual es mayor a la max_priority actual encontrada (este valor se inicializa en -1 ya que la `MIN_PRIORITY` es 0); de ser así, modifica el valor de max_priority a la del proceso actual y apunta el puntero de la variable max_env hacia el proceso actual. En caso de que el proceso actual tenga la misma prioridad que el max_env, el mecanismo de desempate es ver si el proceso actual tiene un número de env_runs menor al del max_ev, así garantizando cierta medida de fairness en nuestro scheduling. Este proceso de búsqueda es de tiempo lineal en cualquier caso. Como mejora, podríamos proponer fácilmente una implementación de un heap binario para recurrir al proceso de mayor prioridad y menor cantidad de runs en tiempo constante (y lograr inserciones y eliminaciones en tiempo logarítmico).

<pre><code>
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
</code></pre>

Hasta ahora describimos el proceso de selección del próximo proceso basado en su prioridad, pero todavía nos falta describir como es que esta prioridad decrece a medida que el proceso suma env_runs (es decir a medida que hace más uso de los distintos CPU). Para lidiar con el decremento de la prioridad de los procesos, incluimos otro miembro al struct Env llamado priority_counter. Este atributo del proceso es tenido en cuenta al momento de ser elegido para correr por el scheduler. Simplemente, por cada nueva ejecución del proceso, el priority_counter es incrementado por 1. Al alcanzar el valor de `PRIORITY_TOLERANCE` en su priority_counter, la prioridad del proceso (env->priority) es decrementada por 1 para priorizar la preferencia del scheduler por procesos ejecutados menos veces, así hasta llegar a 0, la `MIN_PRIORITY`, estado irreducible.

<pre><code>if (max_env) {
		env_statistics.env_rans++;
		max_env->priority_counter++;
		if (max_env->priority_counter == PRIORITY_TOLERANCE) {
			if (max_env->priority > MIN_PRIORITY) {
				max_env->priority--;
			}
			max_env->priority_counter = 0;
		}
		env_run(max_env);
	}</code></pre>

Ahora bien, sólo nos faltaría resolver el problema de la inanición o "starvation" de procesos. Esto es cuando procesos de baja prioridad de ejecución son relegados por el scheduler ante la alta presencia de procesos de máxima o mayor prioridad (decimos máxima ya que en una situación de scheduling real, las operaciones bloqueantes son grandes aliadas para mantener a los procesos en prioridades altas). Nuestra solución es que cada cierto número de ejecuciones, determinado por la variable `BOOST_INTERVAL`, se ejecuta `envs_boost_priority()`, una función que aumenta la prioridad de todos los entornos en estado ENV_RUNNABLE. Este mecanismo puede ayudar a evitar la "starvation" pero realmente para procurar una escala de fairness mayor en el scheduler, deberíamos considerar implementaciones más complejas que contemplen no solo eficiencia sino justicia, como lo es un CFS.

<pre><code>
if (env_statistics.env_rans % BOOST_INTERVAL == 0) {
    envs_boost_priority();
}
</code></pre>
