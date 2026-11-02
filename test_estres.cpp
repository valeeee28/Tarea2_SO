
#include "juez.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    constexpr int TOTAL_PARTIDAS = 30;
    constexpr int PARTIDAS_SIMULTANEAS = 4;

    std::atomic<int> aprobadas{0};
    std::atomic<int> fallidas{0};
    std::atomic<int> movimientosTotales{0};
    std::atomic<int> capturasTotales{0};

    auto inicio = std::chrono::steady_clock::now();

    auto ejecutarPrueba = [&](int numero) {
        ConfigSimulacion config;

        config.semilla = 1000 + numero;
        config.retardoMinMs = 1;
        config.retardoMaxMs = 2;
        config.duracionMaxMs = 3000;
        config.imprimirTableroFinal = false;

        ResultadoJuez resultado = ejecutarJuez(
            numero,
            config
        );

        bool estadoValido =
            resultado.estado == PARTIDA_GANA_PUMA ||
            resultado.estado == PARTIDA_GANAN_PERROS ||
            resultado.estado == PARTIDA_SIN_GANADOR;

        bool capturasValidas =
            resultado.capturas >= 0 &&
            resultado.capturas <= 12;

        bool hilosValidos =
            resultado.hilosCreados == 13;

        bool movimientosValidos =
            resultado.movimientos >= 0;

        bool victoriaPumaValida =
            resultado.estado != PARTIDA_GANA_PUMA ||
            resultado.capturas >= 6;

        bool valida =
            estadoValido &&
            capturasValidas &&
            hilosValidos &&
            movimientosValidos &&
            victoriaPumaValida;

        if (valida) {
            aprobadas.fetch_add(1);
        } else {
            fallidas.fetch_add(1);
        }

        movimientosTotales.fetch_add(
            resultado.movimientos
        );

        capturasTotales.fetch_add(
            resultado.capturas
        );

        std::cout
            << "Partida " << numero
            << ": " << (valida ? "APROBADA" : "FALLIDA")
            << " | Movimientos: " << resultado.movimientos
            << " | Capturas: " << resultado.capturas
            << " | Hilos: " << resultado.hilosCreados
            << " | Estado: " << resultado.estado
            << "\n";
    };

    for (int inicioLote = 1;
         inicioLote <= TOTAL_PARTIDAS;
         inicioLote += PARTIDAS_SIMULTANEAS) {

        std::vector<std::thread> hilos;

        for (int i = inicioLote;
             i < inicioLote + PARTIDAS_SIMULTANEAS &&
             i <= TOTAL_PARTIDAS;
             i++) {

            hilos.emplace_back(ejecutarPrueba, i);
        }

        for (auto& hilo : hilos) {
            hilo.join();
        }
    }

    auto fin = std::chrono::steady_clock::now();

    auto duracion =
        std::chrono::duration_cast<
            std::chrono::milliseconds
        >(fin - inicio).count();

    std::cout << "\n=== RESULTADO DE ESTRES ===\n";
    std::cout << "Partidas: " << TOTAL_PARTIDAS << "\n";
    std::cout << "Aprobadas: " << aprobadas.load() << "\n";
    std::cout << "Fallidas: " << fallidas.load() << "\n";
    std::cout << "Movimientos totales: "
              << movimientosTotales.load() << "\n";
    std::cout << "Capturas totales: "
              << capturasTotales.load() << "\n";
    std::cout << "Tiempo total: "
              << duracion << " ms\n";

    bool resultadoFinal =
        fallidas.load() == 0 &&
        movimientosTotales.load() >= 500;

    std::cout
        << (resultadoFinal
            ? "PRUEBA DE ESTRES APROBADA"
            : "PRUEBA DE ESTRES FALLIDA")
        << "\n";

    return resultadoFinal ? 0 : 1;
}
