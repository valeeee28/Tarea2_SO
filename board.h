#ifndef BOARD_H
#define BOARD_H
#include <pthread.h>
#include <vector>

constexpr int FILAS=5, COLUMNAS=5, TOTAL_CUADRICULA=25;
enum PosicionTrono { TRONO_LEFT=25, TRONO_TOP=26, TRONO_RIGHT=27 };

enum Casilla { VACIA=0, PUMA=1, PERRO=2 };

struct Posicion { int fila; int columna; };

struct Tablero {
    Casilla celdas[FILAS][COLUMNAS];
    std::vector<Casilla> trono;
    int perrosCapturados;
    pthread_mutex_t mutex;
};

void inicializarTablero(Tablero& tablero);

void mostrarTablero(Tablero& tablero);

void destruirTablero(Tablero& tablero);

bool posicionValida(int fila,int columna);

bool movimientoValido(Tablero& tablero,Posicion origen,Posicion destino);

bool moverFicha(Tablero& tablero,Posicion origen,Posicion destino);

bool capturarPerro(Tablero& tablero,Posicion origen,Posicion destino);

bool victoriaPuma(Tablero& tablero);

bool pumaTieneMovimientos(Tablero& tablero);

bool victoriaPerros(Tablero& tablero);

bool colocarFicha(Tablero& tablero,Posicion p,Casilla ficha);
#endif
