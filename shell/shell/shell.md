# shell

### Búsqueda en $PATH

---
¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La principal diferencia entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3) es que la primera es una syscall directa del kernel. Al hallarse en un nivel más bajo con respecto al kernel, requiere de especificaciones más estrictas como una ruta completa del ejecutable, lo que la hace menos flexible y más propensa a fallar en su objetivo. El pathname debe ser un binario ejecutable o un script que de la forma #!interpreter [optional-arg] mientras que la familia de wrappers exec(3) puede recibir simplemente un nombre de archivo ejecutable y realizar una búsqueda en PATH del binario ejecutable (en algunas de sus variantes), buscar en el entorno actual de ejecución o recibir un nuevo entorno por parámetro. Además, estas funciones son capaces de recibir la lista de argumentos y el entorno del programa en distintos formatos, ya sea en forma de vector o lista de strings, lo que las hacen más adecuadas para distintas necesidades del programador. Cabe aclarar que la familia exec(3) envuelve a la syscall execve(2), es decir, la ejecuta internamente.

¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

La llamada a exec(3) puede fallar en distintas ocasiones, como por ejemplo si el ejecutable no es encontrado (generalmente por la ausencia de un #! al principio de un script) o por errores de memoria. La realidad es que las funciones de exec(3) pueden fallar por todas las mismas razones que execve(), ya que es la función que realiza en última instancia la acción de exec por la razón que mencionamos anteriormente, son wrappers de esta syscall y no extienden su comportamiento más allá de los parámetros recibidos. Estos errores, determinados por errno, son:

- EACESS: El argumento nombre de ruta no se refiere a un archivo normal, el archivo no tiene el permiso de ejecución habilitado o uno de los componentes del directorio de nombre de ruta no se puede buscar.
- ENOENT: El archivo referido por la ruta no existe.
- ENOEXEC: El archivo referido por la ruta está marcado como ejecutable pero no está en un formato ejecutable reconocible. Posiblemente sea un script que empiece con #!.
- ETXTBSY: El archivo referido por la ruta está abierto para escritura en otro proceso.
- E2BIG: El espacio total requerido por la lista de argumentos y entorno excede el máximo permitido.

En estos casos, la implementación de la shell captura el error, muestra el mensaje de error correspondiente al usuario por stderr y termina el proceso con un llamado a exit, devolviendo así el control al proceso shell.

### Procesos en segundo plano

---

### Flujo estándar
Investigar el significado de 2>&1, explicar cómo funciona su forma general
Mostrar qué sucede con la salida de cat out.txt en el ejemplo.
Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).

### Respuesta:
El significado de 2>&1 es la redirección desde el flujo de error estándar (stderr) hacia el flujo de salida estándar (stdout). Es decir, el 2>&1 combina ambos flujos para que el error también salga por el mismo flujo que la salida.

En el caso de salida de cat out.txt está el caso de poner 2>&1 >out.txt y el caso de >out.txt 2>&1. En el primer caso, devuelve un "noexiste: No such file or directory" porque el stderr y el stdout se combinan en el flujo y al out.txt se redirige al flujo de salida pero el stderr sigue en esa terminal. En el segundo caso, primero se redirige el out.txt al stdout y después se redirige el stderr a stdout donde esta el out.txt. Entonces la terminal no va a transmitir ningún mensaje porque ambos flujos fueron redirigidos al archivo. El comportamiento del bash es exactamente el mismo al comportamiento en esta situación.

### Tuberías múltiples
Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
¿Cambia en algo?
¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.

### Respuesta:
Lo que ocurre con el exit code reportado por la shell si se ejecuta un pipe es que se ejecuta el último comando mandado por el pipe.

Si alguno de los comandos falla pero no es el último, se oculta ese fallo y se ejecuta el último comando pasado por terminal. Por ejemplo, si se ejecuta
ls /noexiste | echo "Archivo no encontrado"
echo $?
el comando /noexiste mandaria un error pero como se muestra el echo ejecuta ee sin problemas. En nuestra implementación y la del bash se comporta de la misma forma y ejecuta el último comando pasado por la terminal.



### Variables de entorno temporarias

Responder: ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

#### Respuesta:
Es necesario hacerlo luego de la llamada a fork(2) porque el proceso hijo hereda las variables de entorno del proceso padre, por lo que si se cambian las variables de entorno antes de hacer la llamada a fork(2), el proceso hijo heredará las variables de entorno modificadas.
Y esto pasa para todos los hijos posteriores. Cuando en este caso solo queremos que las variables queden para este proceso.
Un caso que demuestra cómo debería funcionar es el siguiente:

	bash
	$  MI_VARIABLE=Hola env // Imprimirá todas las variables de entorno mas la nueva variable MI_VARIABLE
	MI_VARIABLE=Hola
	SHELL=/bin/bash
	...
	$ MI_VARIABE=Hola env | env // No se imprimirá la variable MI_VARIABLE.
	SHELL=/bin/bash
	...
Esto sucede porque la variable MI_VARIABLE solo se encuentra en el proceso hijo, y no en el proceso padre. En caso de estar en el proceso padre, está dejaría de ser Temporal.



#### Responder:
En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
#### Respuesta:
No, el comportamiento no es el mismo, ya que si le pasamos las variables de entorno al exec(3) el entorno del proceso hijo será SOLO las variables que se le pasó, sin las del proceso padre. En cambio si se usa setenv(3) se le agregan las variables al entorno del proceso hijo, sin eliminar las del proceso padre.

#### Responder:
Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

#### Respuesta:
Una posible implementación para que el comportamiento sea el mismo sería guardar las variables de entorno del proceso padre en un arreglo, y luego agregar las variables que se quieren agregar al proceso hijo. Luego, se le pasa este arreglo al exec(3) para que el proceso hijo tenga las variables de entorno del proceso padre y las nuevas variables que se quieren agregar.

---

### Pseudo-variables
#### Respuesta:

Las variables mágicas, o pseudo-variables, son variables especiales a las cuales se puede acceder, pero no se pueden modificar.


##### Variable 0: $?
Esta variable contiene el valor de retorno del último comando ejecutado. Si el comando se ejecutó correctamente, el valor de retorno es 0, si no, es un número distinto de 0.
Ejemplo de uso:


	$ /bin/true // La salida de este comando siempre es 0
	$ echo $? // Como el comando anterior se ejecutó correctamente, la salida de este comando es 0
	0
    
	$ /bin/false // La salida de este comando siempre es 1
	$ echo $? // Como el comando anterior no se ejecutó correctamente, la salida de este comando es 1
	1


##### Variable 1: $$
Esta variable contiene el PID de la Shell, en caso de estar un una subshell, este comando devolverá el PID, de la shell actual.
Ejemplo de uso:

	bash
	$ echo $$ // Muestra el PID del proceso actual
	1234

##### Variable 2: $0
Esta variable contiene el nombre del script que se está ejecutando. En caso de que bash se ejecuta a través de un archivo, esta variable contendrá el nombre del archivo.
Ejemplo de uso:

	$ echo $0 // Muestra el nombre del script que se está ejecutando
	-bash

##### Variable 3: $#
Esta variable contiene la cantidad de argumentos que se pasaron al script.
Ejemplo de uso:

    	$ echo $# // Muestra la cantidad de argumentos que se pasaron al script
    	0 // Al ejecutarse en la shell, no se pasan argumentos

---


### Comandos built-in
#### Responder:
¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)
#### Respuesta:
En el caso de cd no se podría implementar sin ser built-in porque los procesos hijos no tienen la capacidad de modificar el entorno del proceso padre. En cambio, el pwd si sería posible ya que su única función sería mostrar el directorio actual sin la necesidad de modificar. Al pensar en los built-in como true y false (0 y 1), la razón principal para que pwd se built-in es la eficiencia ya que evitan la creación de nuevos procesos en algo simple y constantemente usado.

---
### Procesos en segundo plano
#### Responder:
Explicar detalladamente el mecanismo completo utilizado.
¿Por qué es necesario el uso de señales?
#### Respuesta:

El mecanismo utilizado consiste en la modificación del manejo por defecto de la señal SIGCHLD (señal que el proceso hijo le envía al padre cuando se frena o termina), por medio de la implementación de un handler del lado del user-space. El comportamiento default del sistema operativo es ignorar dicha señal, lo que abre la posibilidad de que los procesos hijos se conviertan en procesos zombie, dado que hasta que su padre no llame a wait o waitpid su estado no se limpiará. Para nuestra implementación, creamos un handler específico para esta señal como una función de C. Lo que realiza el handler es simplemente esperar por la terminación de cualquier proceso hijo del proceso principal (shell) que comparta PGID (Process-Group ID) con su padre, imprimiendo por salida estándar (debug) el mensaje de terminación de cada proceso. El handler es configurado en el init, donde todavía no hay ningún proceso, para que todos los hijos hereden nuestro handler (todos los procesos creados o sustituidos a través de fork o execve heredan su pgid). Además, para garantizar que esto funcione, modificamos el PGID de los procesos en primer plano en runcmd.c con setpgid(0, 0), logrando que los procesos en segundo plano sean los únicos que compartan PGID con la shell.

Previo a esto, requerimos de un setup inicial de este handler, lo que involucró la asignación de memoria dinámica de un stack de ejecución alternativo, distinto al stack del usuario, en el que delegar la ejecución de nuestro handler, a través de la syscall sigaltstack, la cual provee una interfaz para dicho propósito. Esto es muy recomendable ya que aísla al stack de usuario de cualquier error proveniente de procesos ejecutados en el background, además de resguardarlo de un stack overflow. Para el setup del handler utilizamos la syscall sigaction, con la que establecemos el stack alternativo mediante la flag SA_ONSTACK y el handler a través del atributo sa_handler del struct sigaction. Además se le proveen las flags SA_RESTART y SA_NOCLDSTOP para asegurar que las llamadas al sistema interrumpidas por la señal se reinicien automáticamente y para evitar que se genere una señal SIGCHLD cuando un proceso hijo se detiene, respectivamente.

Es necesario el uso de señales para que cuando el proceso en segundo plano termine notifique al kernel o al handler actual, que este proceso terminó. Esto nos da la ventaja que el proceso padre no tenga que quedar bloqueado esperando a que el proceso en segundo plano termine. Por el contrario, el proceso padre continúa con otras tareas y es notificado mediante una señal en el momento exacto que un proceso hijo termina. Además, evita que los procesos hijos se conviertan en zombies, dado que el handler se encarga de limpiar su estado después de recibir la señal. Las señales son muy necesarias ya que facilitan la comunicación y coordinación entre procesos, lo que es de gran utilidad en sistemas que manejan múltiples hilos de forma sincrónica y asincrónica.

