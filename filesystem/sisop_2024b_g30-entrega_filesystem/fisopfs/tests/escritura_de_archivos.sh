VERDE="\e[0;32m"
ROJO="\e[0;32m"
RESET="\e[0m"

# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi
echo "Nuevo contenido" > archivo9.txt 2> /dev/null
echo "Contenido adicional" >> archivo9.txt 2> /dev/null

if grep -q "Nuevo contenido" archivo9.txt && grep -q "Contenido adicional" archivo9.txt; then
    echo -e "Escritura de archivos: $VERDE PASÓ $RESET"
    rm archivo7.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Escritura de archivos: $ROJO FALLÓ $RESET"
    rm archivo7.txt > /dev/null 2>&1
    exit 1
fi