
#ifndef SIMULACION_H
#define SIMULACION_H

#include "board.h"

struct ConfigSimulacion {
    unsigned int semilla = 42;
    int retardoMinMs = 50;
    int retardoMaxMs = 300;
    int duracionMaxMs = 5000;
    bool imprimirTableroFinal = true;
};

struct ResultadoSimulacion {
    int movimientos = 0;
    int capturas = 0;
    int ganador = 0; 
    int hilosCreados = 0;
};

ResultadoSimulacion simularPartida(
    Tablero& tablero,
    const ConfigSimulacion& config
);

#endif
