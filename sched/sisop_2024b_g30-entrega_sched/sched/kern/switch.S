
/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */


# ||
# ||
# |	trapframe	|
# |direccion de retorno| 

.globl context_switch;
context_switch:
	# De modo Kernel a modo usuario
	
	# Hacemos que el stack pointer apunte al trapframe que es recibido por parametros
	add $4, %esp # Saltamos el return address
	mov (%esp), %esp # apuntamos al trapframe

	popal           # Restaurar tf_regs
	pop %ds         # Restaurar data segment
	pop %es         # Restaurar extra segment

	add	$4,%esp		# Sacar el trapframe de la pila
	add $4,%esp		# Sacar el trapframe de la pila

	iret			# Salir del modo kernel

	
spin:
	jmp spin
