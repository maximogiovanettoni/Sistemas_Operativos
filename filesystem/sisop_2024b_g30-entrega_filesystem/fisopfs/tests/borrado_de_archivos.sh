#!/bin/bash

# Colores para salida
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"

# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi

# Intentar borrar archivos
touch archivo_a_borrar_1.txt
rm archivo_a_borrar_1.txt 2> /dev/null
RESULTADO=$? # Guardar resultado inicial
touch archivo_a_borrar_2.txt
rm archivo_a_borrar_2.txt 2> /dev/null
RESULTADO=$(( RESULTADO + $? )) # Sumar resultado del segundo `rm`

# Verificar resultado de borrado
if [ $RESULTADO -eq 0 ]; then
    echo -e "Borrado de archivos: ${VERDE}PASÓ${RESET}"
    exit 0
else
    echo -e "Borrado de archivos: ${ROJO}FALLÓ${RESET}"
    exit 1
fi
