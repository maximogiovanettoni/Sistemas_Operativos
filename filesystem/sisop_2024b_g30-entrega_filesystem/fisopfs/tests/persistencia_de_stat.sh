VERDE="\e[0;32m"
ROJO="\e[0;32m"
RESET="\e[0m"

# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi
echo prueba > archivo24.txt

STAT_1=$(stat archivo24.txt)
RESULTADO=$?

echo cambios > archivo24.txt
RESULTADO=$(( $RESULTADO + $? ))

STAT_2=$(stat archivo24.txt)
RESULTADO=$(( $RESULTADO + $? ))

if [ "$STAT_1" != "$STAT_2" ] && [ $RESULTADO -eq 0 ]; then
    echo -e "Cambio de stats de archivos al editar: $VERDE PASÓ $RESET"
    rm archivo24.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Cambio de stats de archivos al editar: $ROJO FALLÓ $RESET"
    rm archivo24.txt > /dev/null 2>&1
    exit 1
fi