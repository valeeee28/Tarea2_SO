
#ifndef BOARD_H
#define BOARD_H

#include <pthread.h>
#include <vector>

constexpr int FILAS = 5;
constexpr int COLUMNAS = 5;
constexpr int TOTAL_CUADRICULA = FILAS * COLUMNAS;

enum Casilla {
    VACIA = 0,
    PUMA = 1,
    PERRO = 2
};

struct Posicion {
    int fila;
    int columna;
};

struct Tablero {
    Casilla celdas[FILAS][COLUMNAS];

    std::vector<Casilla> trono;

    int perrosCapturados;

    pthread_mutex_t mutex;
};

void inicializarTablero(Tablero& tablero);
void mostrarTablero(Tablero& tablero);
void destruirTablero(Tablero& tablero);

#endif 
 
