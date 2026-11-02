
#include "juez.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    ConfigSimulacion config;
    config.semilla = 42;
    config.retardoMinMs = 20;
    config.retardoMaxMs = 50;
    config.duracionMaxMs = 500;
    config.imprimirTableroFinal = false;

    bool correcto = true;

    std::cout << "=== PRUEBA 1: PARTIDA NORMAL ===\n";

    std::atomic<bool> detenerNormal{false};

    ResultadoJuez normal = ejecutarJuez(
        1, config, &detenerNormal
    );

    bool prueba1 =
        normal.hilosCreados == 13 &&
        normal.estado != PARTIDA_ERROR &&
        normal.estado != PARTIDA_CANCELADA;

    std::cout << (prueba1 ? "APROBADA\n" : "FALLIDA\n");
    correcto = correcto && prueba1;

    std::cout << "\n=== PRUEBA 2: CANCELACION PREVIA ===\n";

    std::atomic<bool> detenerAntes{true};

    ResultadoJuez previa = ejecutarJuez(
        2, config, &detenerAntes
    );

    bool prueba2 =
        previa.estado == PARTIDA_CANCELADA &&
        previa.hilosCreados == 0;

    std::cout << (prueba2 ? "APROBADA\n" : "FALLIDA\n");
    correcto = correcto && prueba2;

    std::cout << "\n=== PRUEBA 3: CANCELACION ACTIVA ===\n";

    std::atomic<bool> detenerDurante{false};
    ResultadoJuez cancelada;

    config.duracionMaxMs = 10000;
    config.retardoMinMs = 50;
    config.retardoMaxMs = 100;

    std::thread hilo([&]() {
        cancelada = ejecutarJuez(
            3, config, &detenerDurante
        );
    });

    std::this_thread::sleep_for(
        std::chrono::milliseconds(200)
    );

    auto inicio = std::chrono::steady_clock::now();

    detenerDurante.store(true);
    hilo.join();

    auto demora =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - inicio
        ).count();

    bool prueba3 =
        cancelada.estado == PARTIDA_CANCELADA &&
        cancelada.hilosCreados == 13 &&
        demora < 2000;

    std::cout << "Hilos creados: "
              << cancelada.hilosCreados << "\n";

    std::cout << "Tiempo de cierre: "
              << demora << " ms\n";

    std::cout << (prueba3 ? "APROBADA\n" : "FALLIDA\n");
    correcto = correcto && prueba3;

    std::cout << "\n=== RESULTADO FINAL ===\n";

    std::cout << (
        correcto
        ? "TODAS LAS PRUEBAS DE DETENCION APROBADAS\n"
        : "HAY PRUEBAS FALLIDAS\n"
    );

    return correcto ? 0 : 1;
}
