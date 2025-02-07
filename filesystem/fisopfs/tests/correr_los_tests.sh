#!/bin/bash

VERDE="\e[0;32m"
ROJO="\e[0;31m"
AMARILLO="\e[0;33m"
RESET="\e[0m"


TEST_DIR="." 

tests=(
"crear_un_archivo.sh"
"borrado_de_archivos.sh"
"crear_dirs_en_distintos_niveles.sh"
"escritura_de_archivos.sh"
"crear_un_dir.sh"
"borrado_de_directorios.sh"
"persistencia_de_stat.sh"
"cambio_stats.sh"
"archivo_largo.sh"
)

pasaron=0
fallaron=0

for test in "${tests[@]}"; do
    test_path="${TEST_DIR}/${test}"

    if [[ ! -f "$test_path" ]]; then
        echo -e "${ROJO}Error: Test script $test not found${RESET}"
        fallaron=$((fallaron + 1))
        continue
    fi

    chmod +x "$test_path"

    "$test_path"
    resultado=$?

    if [[ $resultado -eq 0 ]]; then
        echo -e "${VERDE}Test $test pasó${RESET}"
        pasaron=$((pasaron + 1))
    else
        echo -e "${ROJO}Test $test falló${RESET}"
        fallaron=$((fallaron + 1))
    fi
done

echo -e "\n${AMARILLO}Resumen de los tests\n================$RESET"
echo -e "${VERDE}Pasaron: $pasaron $RESET"
echo -e "${ROJO}Fallaron: $fallaron $RESET"

[[ $fallaron -eq 0 ]] || exit 1