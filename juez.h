
#ifndef JUEZ_H
#define JUEZ_H

#include "simulacion.h"
#include <atomic>

enum EstadoPartida {
    PARTIDA_ERROR = -1,
    PARTIDA_SIN_GANADOR = 0,
    PARTIDA_GANA_PUMA = 1,
    PARTIDA_GANAN_PERROS = 2,
    PARTIDA_CANCELADA = 3
};

enum AbandonoJugador {
    SIN_ABANDONO = 0,
    ABANDONA_PUMA = 1,
    ABANDONAN_PERROS = 2
};

struct ResultadoJuez {
    EstadoPartida estado = PARTIDA_ERROR;
    int movimientos = 0;
    int capturas = 0;
    int hilosCreados = 0;
    bool victoriaPorAbandono = false;
};

ResultadoJuez ejecutarJuez(
    int idMesa,
    const ConfigSimulacion& config,
    const std::atomic<bool>* detenerExterno = nullptr
);

ResultadoJuez ejecutarJuez(
    int idMesa,
    const ConfigSimulacion& config,
    const std::atomic<bool>* detenerExterno,
    const std::atomic<int>* abandonoJugador
);

#endif
