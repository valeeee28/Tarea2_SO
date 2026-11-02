
#include "juez.h"

#include <iostream>
#include <pthread.h>

namespace {

struct DatosJuez {
    int idMesa;
    ConfigSimulacion config;
    const std::atomic<bool>* detenerExterno;
    ResultadoJuez resultado;
};

bool prepararTablero(Tablero& tablero) {
    if (!colocarFicha(tablero, {2, 2}, PUMA)) {
        return false;
    }

    for (int columna = 0; columna < 5; columna++) {
        if (!colocarFicha(
            tablero, {0, columna}, PERRO
        )) return false;

        if (!colocarFicha(
            tablero, {4, columna}, PERRO
        )) return false;
    }

    return colocarFicha(tablero, {1, 0}, PERRO) &&
           colocarFicha(tablero, {3, 4}, PERRO);
}

void* funcionJuez(void* argumento) {
    auto* datos = static_cast<DatosJuez*>(argumento);

    if (datos->detenerExterno &&
        datos->detenerExterno->load()) {
        datos->resultado.estado = PARTIDA_CANCELADA;
        return nullptr;
    }

    Tablero tablero;
    inicializarTablero(tablero);

    if (!prepararTablero(tablero)) {
        destruirTablero(tablero);
        return nullptr;
    }

    ResultadoSimulacion simulacion = simularPartida(
        tablero,
        datos->config,
        datos->detenerExterno
    );

    datos->resultado.movimientos =
        simulacion.movimientos;

    datos->resultado.capturas =
        simulacion.capturas;

    datos->resultado.hilosCreados =
        simulacion.hilosCreados;

    switch (simulacion.motivo) {
        case FIN_VICTORIA_PUMA:
            datos->resultado.estado = PARTIDA_GANA_PUMA;
            break;

        case FIN_VICTORIA_PERROS:
            datos->resultado.estado = PARTIDA_GANAN_PERROS;
            break;

        case FIN_TIEMPO:
            datos->resultado.estado = PARTIDA_SIN_GANADOR;
            break;

        case FIN_CANCELACION:
            datos->resultado.estado = PARTIDA_CANCELADA;
            break;

        default:
            datos->resultado.estado = PARTIDA_ERROR;
            break;
    }

    destruirTablero(tablero);
    return nullptr;
}

} // namespace

ResultadoJuez ejecutarJuez(
    int idMesa,
    const ConfigSimulacion& config,
    const std::atomic<bool>* detenerExterno
) {
    ResultadoJuez error;

    DatosJuez datos{};
    datos.idMesa = idMesa;
    datos.config = config;
    datos.detenerExterno = detenerExterno;

    pthread_t hiloJuez;

    if (pthread_create(
        &hiloJuez,
        nullptr,
        funcionJuez,
        &datos
    ) != 0) {
        std::cerr << "Error creando juez de mesa "
                  << idMesa << "\n";
        return error;
    }

    pthread_join(hiloJuez, nullptr);

    return datos.resultado;
}
