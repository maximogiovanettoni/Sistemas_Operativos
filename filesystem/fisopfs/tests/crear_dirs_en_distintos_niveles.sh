
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"


# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi
CREADO_EL_DIR=$PWD

mkdir -p directorio1/directorio2/directorio3 2> /dev/null
cd directorio1/directorio2/directorio3 2> /dev/null

touch archivo4.txt 2> /dev/null
RESULTADO=$?

if [ -f "archivo4.txt" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Crear directorios en distintos : $VERDE PASÓ $RESET"
    cd $CREADO_EL_DIR
    rm -rf directorio1 > /dev/null 2>&1
    exit 0
else
    echo -e "Multiples niveles: $ROJO FALLÓ $RESET"
    cd $CREADO_EL_DIR
    rm -rf directorio1 > /dev/null 2>&1
    exit 1
fi