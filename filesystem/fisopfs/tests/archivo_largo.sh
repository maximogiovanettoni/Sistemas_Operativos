
VERDE="\e[0;32m"
ROJO="\e[0;31m"
RESET="\e[0m"


TEXTO_LARGO="La va a tocar para Diego, ahí la tiene Maradona, lo marcan dos, pisa la pelota Maradona, arranca por la derecha el genio del fútbol mundial, y deja el tendal y va a tocar para Burruchaga… ¡Siempre Maradona! ¡Genio! ¡Genio! ¡Genio! Ta-ta-ta-ta-ta-ta… Goooooool… Gooooool… ¡Quiero llorar! ¡Dios santo, viva el fútbol! ¡Golaaaaaaazooooooo! ¡Diegooooooool! ¡Maradona! Es para llorar, perdónenme… Maradona, en una corrida memorable, en la jugada de todos los tiempos… barrilete cósmico… ¿De qué planeta viniste? ¡Para dejar en el camino a tanto inglés! ¡Para que el país sea un puño apretado, gritando por Argentina!.. Argentina 2 – Inglaterra 0… Diegol, Diegol, Diego Armando Maradona… Gracias Dios, por el fútbol, por Maradona, por estas lágrimas, por este Argentina 2 – Inglaterra 0"

# Cambiar a directorio ../prueba
if ! cd ../prueba 2> /dev/null; then
    echo -e "${ROJO}Error: No se pudo acceder al directorio ../prueba${RESET}"
    exit 1
fi


echo "$TEXTO_LARGO" > archivo_largo.txt 2> /dev/null


if grep -q "$TEXTO_LARGO" archivo_largo.txt; then
    echo -e "Cuando se crea un archivo largo: ${VERDE}PASÓ${RESET}"
    rm archivo_largo.txt > /dev/null 2>&1
    exit 0
else
    echo -e "Cuando se crea un archivo largo: ${ROJO}FALLÓ${RESET}"
    rm archivo_largo.txt > /dev/null 2>&1
    exit 1
fi
