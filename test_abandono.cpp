
#include "juez.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    int aprobadas = 0;

    ConfigSimulacion config;
    config.retardoMinMs = 50;
    config.retardoMaxMs = 300;
    config.duracionMaxMs = 10000;
    config.imprimirTableroFinal = false;

    auto comprobar = [&](bool correcto, const char* nombre) {
        std::cout << nombre << ": "
                  << (correcto ? "APROBADA" : "FALLIDA")
                  << "\n";

        if (correcto) aprobadas++;
    };

    {
        std::atomic<int> abandono{ABANDONA_PUMA};

        ResultadoJuez r = ejecutarJuez(
            1, config, nullptr, &abandono
        );

        comprobar(
            r.estado == PARTIDA_GANAN_PERROS &&
            r.victoriaPorAbandono,
            "Abandona Puma: ganan Perros"
        );
    }

    {
        std::atomic<int> abandono{ABANDONAN_PERROS};

        ResultadoJuez r = ejecutarJuez(
            2, config, nullptr, &abandono
        );

        comprobar(
            r.estado == PARTIDA_GANA_PUMA &&
            r.victoriaPorAbandono,
            "Abandonan Perros: gana Puma"
        );
    }

    {
        std::atomic<int> abandono{SIN_ABANDONO};
        ResultadoJuez r;

        std::thread hilo([&]() {
            r = ejecutarJuez(
                3, config, nullptr, &abandono
            );
        });

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );

        abandono.store(ABANDONA_PUMA);
        hilo.join();

        comprobar(
            r.estado == PARTIDA_GANAN_PERROS &&
            r.victoriaPorAbandono &&
            r.hilosCreados == 13,
            "Abandono durante partida activa"
        );
    }

    {
        std::atomic<bool> detener{true};
        std::atomic<int> abandono{ABANDONA_PUMA};

        ResultadoJuez r = ejecutarJuez(
            4, config, &detener, &abandono
        );

        comprobar(
            r.estado == PARTIDA_CANCELADA &&
            !r.victoriaPorAbandono,
            "SIGINT tiene prioridad"
        );
    }

    std::cout
        << "\nRESULTADO: "
        << aprobadas
        << "/4 PRUEBAS APROBADAS\n";

    return aprobadas == 4 ? 0 : 1;
}
