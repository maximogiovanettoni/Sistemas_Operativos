#!/bin/bash

# Colores para salida
VERDE="\e[0;32m"
ROJO="\e[0;31m"  # Cambiado el color rojo correctamente
RESET="\e[0m"

# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi

# Intentar borrar el directorio
rmdir directorio 2> /dev/null
RESULTADO=$?

# Verificar si el directorio fue eliminado exitosamente
if [ $RESULTADO -eq 0 ] && [ ! -d "directorio" ]; then
    echo -e "Borrar el directorio: ${VERDE}PASÓ${RESET}"
    exit 0
else
    echo -e "Borrar el directorio: ${ROJO}FALLÓ${RESET}"
    exit 1
fi
