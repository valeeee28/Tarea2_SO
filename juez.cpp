
#include "juez.h"

#include <iostream>
#include <pthread.h>
#include <chrono>
#include <thread>

namespace {

struct DatosJuez {
    int idMesa;
    ConfigSimulacion config;
    const std::atomic<bool>* detenerExterno;
    const std::atomic<int>* abandonoJugador;
    ResultadoJuez resultado;
};

bool prepararTablero(Tablero& tablero) {
    if (!colocarFicha(tablero, {2, 2}, PUMA))
        return false;

    for (int columna = 0; columna < 5; columna++) {
        if (!colocarFicha(tablero, {0, columna}, PERRO))
            return false;

        if (!colocarFicha(tablero, {4, columna}, PERRO))
            return false;
    }

    return colocarFicha(tablero, {1, 0}, PERRO) &&
           colocarFicha(tablero, {3, 4}, PERRO);
}

EstadoPartida traducirResultado(
    MotivoFinSimulacion motivo
) {
    switch (motivo) {
        case FIN_VICTORIA_PUMA:
            return PARTIDA_GANA_PUMA;
        case FIN_VICTORIA_PERROS:
            return PARTIDA_GANAN_PERROS;
        case FIN_TIEMPO:
            return PARTIDA_SIN_GANADOR;
        case FIN_CANCELACION:
            return PARTIDA_CANCELADA;
        default:
            return PARTIDA_ERROR;
    }
}

void* funcionJuez(void* argumento) {
    auto* datos = static_cast<DatosJuez*>(argumento);

    auto canceladoServidor = [&]() {
        return datos->detenerExterno != nullptr &&
               datos->detenerExterno->load();
    };

    
    auto abandono = [&]() -> int {
        if (datos->abandonoJugador == nullptr)
            return SIN_ABANDONO;

        int valor = datos->abandonoJugador->load();

        if (valor == ABANDONA_PUMA ||
            valor == ABANDONAN_PERROS)
            return valor;

        return SIN_ABANDONO;
    };


    if (canceladoServidor()) {
        datos->resultado.estado = PARTIDA_CANCELADA;
        return nullptr;
    }

    if (abandono() != SIN_ABANDONO) {
        datos->resultado.estado =
            abandono() == ABANDONA_PUMA
                ? PARTIDA_GANAN_PERROS
                : PARTIDA_GANA_PUMA;

        datos->resultado.victoriaPorAbandono = true;
        return nullptr;
    }

    Tablero tablero;
    inicializarTablero(tablero);

    if (!prepararTablero(tablero)) {
        destruirTablero(tablero);
        return nullptr;
    }

    std::atomic<bool> detenerPartida{false};
    std::atomic<bool> terminarMonitor{false};

    std::thread monitor([&]() {
        while (!terminarMonitor.load()) {
            if (canceladoServidor() ||
                abandono() != SIN_ABANDONO) {
                detenerPartida.store(true);
                break;
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(2)
            );
        }
    });

    ResultadoSimulacion simulacion = simularPartida(
        tablero,
        datos->config,
        &detenerPartida
    );

    terminarMonitor.store(true);
    monitor.join();

    datos->resultado.movimientos =
        simulacion.movimientos;

    datos->resultado.capturas =
        simulacion.capturas;

    datos->resultado.hilosCreados =
        simulacion.hilosCreados;

    datos->resultado.estado =
        traducirResultado(simulacion.motivo);

    if (canceladoServidor()) {
        datos->resultado.estado = PARTIDA_CANCELADA;
    } else if (
        simulacion.motivo == FIN_CANCELACION &&
        abandono() != SIN_ABANDONO
    ) {
        datos->resultado.estado =
            abandono() == ABANDONA_PUMA
                ? PARTIDA_GANAN_PERROS
                : PARTIDA_GANA_PUMA;

        datos->resultado.victoriaPorAbandono = true;
    }

    destruirTablero(tablero);
    return nullptr;
}

}

ResultadoJuez ejecutarJuez(
    int idMesa,
    const ConfigSimulacion& config,
    const std::atomic<bool>* detenerExterno
) {
    return ejecutarJuez(
        idMesa,
        config,
        detenerExterno,
        nullptr
    );
}

ResultadoJuez ejecutarJuez(
    int idMesa,
    const ConfigSimulacion& config,
    const std::atomic<bool>* detenerExterno,
    const std::atomic<int>* abandonoJugador
) {
    ResultadoJuez error;

    DatosJuez datos{};
    datos.idMesa = idMesa;
    datos.config = config;
    datos.detenerExterno = detenerExterno;
    datos.abandonoJugador = abandonoJugador;

    pthread_t hiloJuez;

    if (pthread_create(
        &hiloJuez,
        nullptr,
        funcionJuez,
        &datos
    ) != 0) {
        std::cerr
            << "Error creando juez de mesa "
            << idMesa << "\n";

        return error;
    }

    pthread_join(hiloJuez, nullptr);

    return datos.resultado;
}
