#include "src/matchmaker.h"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    Config config;
    config.delayMinMs = 50;
    config.delayMaxMs = 100;
    config.duracionMaxMs = 10000;

    Matchmaker torneo(2, 1, config);
    torneo.iniciar();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    bool aceptado = torneo.abandonarJugador(1, RolJugador::PUMA);

    std::cout << "Solicitud de abandono: "
              << (aceptado ? "ACEPTADA" : "RECHAZADA")
              << '\n';

    torneo.esperarFinalizacion();
    return 0;
}
