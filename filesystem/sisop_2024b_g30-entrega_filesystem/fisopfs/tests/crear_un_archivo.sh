
VERDE="\e[0;32m"
ROJO="\e[0;32m"
RESET="\e[0m"

# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi
touch archivo_a_crear_1.txt 2> /dev/null
RESULTADO=$?
echo "estoy creando un archivo!" > archivo_a_crear_2.txt 2> /dev/null
RESULTADO=$(( $RESULTADO + $? ))

if [ -f "archivo_a_crear_1.txt" ] && [ -f "archivo_a_crear_2.txt" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Crear un archivo: $VERDE PASÓ $RESET"
    exit 0
else
    echo -e "Crear un archivo: $ROJO FALLÓ $RESET"
    exit 1
fi