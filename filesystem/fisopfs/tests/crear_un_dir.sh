VERDE="\e[0;32m"
ROJO="\e[0;32m"
RESET="\e[0m"


# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi
mkdir directorio 2> /dev/null
RESULTADO=$?

if [ $RESULTADO -eq 0 ] && [ -d "directorio" ]; then
    echo -e "Crear un directorio: $VERDE PASÓ $RESET"
    exit 0
else
    echo -e "Crear un directorio: $ROJO FALLÓ $RESET"
    exit 1
fi