
#include "board.h"
#include <iostream>

void inicializarTablero(Tablero& tablero) {

    pthread_mutex_init(&tablero.mutex, nullptr);

    pthread_mutex_lock(&tablero.mutex);

    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            tablero.celdas[i][j] = VACIA;
        }
    }

    tablero.trono.clear();

    tablero.perrosCapturados = 0;

    pthread_mutex_unlock(&tablero.mutex);
}

void mostrarTablero(Tablero& tablero) {

    pthread_mutex_lock(&tablero.mutex);

    std::cout << "\n--- TABLERO KOMIKAN ---\n";

    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {

            switch (tablero.celdas[i][j]) {
                case VACIA:
                    std::cout << ". ";
                    break;

                case PUMA:
                    std::cout << "P ";
                    break;

                case PERRO:
                    std::cout << "D ";
                    break;
            }
        }

        std::cout << "\n";
    }

    std::cout << "Perros capturados: "
              << tablero.perrosCapturados
              << "\n";

    pthread_mutex_unlock(&tablero.mutex);
}

void destruirTablero(Tablero& tablero) {

    pthread_mutex_destroy(&tablero.mutex);
}
