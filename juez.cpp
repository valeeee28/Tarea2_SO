
#include "juez.h"
#include <iostream>
#include <pthread.h>

namespace {

struct DatosJuez {
    int idMesa;
    ConfigSimulacion config;
    ResultadoJuez resultado;
};

bool prepararTablero(Tablero& tablero) {
    if (!colocarFicha(tablero, {2, 2}, PUMA)) {
        return false;
    }

    for (int columna = 0; columna < 5; columna++) {
        if (!colocarFicha(tablero, {0, columna}, PERRO)) {
            return false;
        }

        if (!colocarFicha(tablero, {4, columna}, PERRO)) {
            return false;
        }
    }

    return colocarFicha(tablero, {1, 0}, PERRO) &&
           colocarFicha(tablero, {3, 4}, PERRO);
}

void* funcionJuez(void* argumento) {
    DatosJuez* datos = static_cast<DatosJuez*>(argumento);

    Tablero tablero;
    inicializarTablero(tablero);

    std::cout << "\n[MESA " << datos->idMesa
              << "] Iniciando partida\n";

    if (!prepararTablero(tablero)) {
        std::cerr << "[MESA " << datos->idMesa
                  << "] Error al preparar tablero\n";

        destruirTablero(tablero);
        return nullptr;
    }

    ResultadoSimulacion simulacion =
        simularPartida(tablero, datos->config);
        

    datos->resultado.movimientos = simulacion.movimientos;
    datos->resultado.capturas = simulacion.capturas;
    datos->resultado.hilosCreados = simulacion.hilosCreados;

    if (simulacion.hilosCreados != 13) {
        datos->resultado.estado = PARTIDA_ERROR;
    } else if (simulacion.ganador == 1) {
        datos->resultado.estado = PARTIDA_GANA_PUMA;
    } else if (simulacion.ganador == 2) {
        datos->resultado.estado = PARTIDA_GANAN_PERROS;
    } else {
        datos->resultado.estado = PARTIDA_SIN_GANADOR;
    }

    std::cout << "[MESA " << datos->idMesa
              << "] Partida finalizada\n";

    destruirTablero(tablero);

    return nullptr;
}

} 

ResultadoJuez ejecutarJuez(
    int idMesa,
    const ConfigSimulacion& config
) {
    DatosJuez datos{};
    datos.idMesa = idMesa;
    datos.config = config;

    pthread_t hiloJuez;

    int error = pthread_create(
        &hiloJuez,
        nullptr,
        funcionJuez,
        &datos
    );

    if (error != 0) {
        std::cerr << "No se pudo crear el juez\n";
        return datos.resultado;
    }

    pthread_join(hiloJuez, nullptr);

    return datos.resultado;
}
