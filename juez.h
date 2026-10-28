
#ifndef JUEZ_H
#define JUEZ_H

#include "simulacion.h"

enum EstadoPartida {
    PARTIDA_ERROR = -1,
    PARTIDA_SIN_GANADOR = 0,
    PARTIDA_GANA_PUMA = 1,
    PARTIDA_GANAN_PERROS = 2
};

struct ResultadoJuez {
    EstadoPartida estado = PARTIDA_ERROR;
    int movimientos = 0;
    int capturas = 0;
    int hilosCreados = 0;
};

ResultadoJuez ejecutarJuez(
    int idMesa,
    const ConfigSimulacion& config
);

#endif
