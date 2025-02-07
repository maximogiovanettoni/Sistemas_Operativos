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

# Crear archivo temporal
touch archivo10.txt 2> /dev/null
if [ $? -ne 0 ]; then
    echo -e "${ROJO}Error: No se pudo crear el archivo archivo10.txt${RESET}"
    exit 1
fi

# Obtener stats del archivo
STAT_1=$(stat archivo10.txt 2> /dev/null)
RESULTADO=$?
STAT_2=$(stat archivo10.txt 2> /dev/null)
RESULTADO=$((RESULTADO + $?))

# Verificar persistencia de stats
if [ "$STAT_1" == "$STAT_2" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Persistencia de stats de archivos: ${VERDE}PASÓ${RESET}"
    rm archivo10.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Persistencia de stats de archivos: ${ROJO}FALLÓ${RESET}"
    rm archivo10.txt > /dev/null 2>&1
    exit 1
fi
