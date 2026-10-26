
#include "board.h"
#include <iostream>

int main() {

    Tablero tablero;

    inicializarTablero(tablero);

    mostrarTablero(tablero);

    destruirTablero(tablero);

    std::cout << "\nPrueba finalizada correctamente.\n";

    return 0;
}
