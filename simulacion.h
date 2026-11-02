
#ifndef SIMULACION_H
#define SIMULACION_H

#include "board.h"
#include <atomic>
#include <array>


struct ConfigSimulacion {
    unsigned int semilla = 42;
    int retardoMinMs = 50;
    int retardoMaxMs = 300;
    int duracionMaxMs = 5000;
    bool imprimirTableroFinal = true;

    bool posicionesPersonalizadas = false;
    Posicion pumaInicial = {2, 2};
    std::vector<Posicion> perrosIniciales;
    int imprimirCadaMovimientos = 20;
};


enum MotivoFinSimulacion {
    FIN_ERROR = -1,
    FIN_TIEMPO = 0,
    FIN_VICTORIA_PUMA = 1,
    FIN_VICTORIA_PERROS = 2,
    FIN_CANCELACION = 3
};

struct ResultadoSimulacion {
    int movimientos = 0;
    int capturas = 0;
    int ganador = 0;
    int hilosCreados = 0;
    MotivoFinSimulacion motivo = FIN_ERROR;
};

ResultadoSimulacion simularPartida(
    Tablero& tablero,
    const ConfigSimulacion& config,
    const std::atomic<bool>* detenerExterno = nullptr
);

#endif
