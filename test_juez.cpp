
#include "juez.h"
#include <iostream>

int main() {

    ConfigSimulacion config;

    config.semilla = 42;
    config.retardoMinMs = 1;
    config.retardoMaxMs = 5;
    config.duracionMaxMs = 1000;
    config.imprimirTableroFinal = false;

    std::cout << "\n=== PRUEBA DEL JUEZ ===\n";

    ResultadoJuez resultado = ejecutarJuez(1, config);

    std::cout << "\n--- RESULTADO MESA 1 ---\n";

    std::cout << "Hilos de fichas: "
              << resultado.hilosCreados << "\n";

    std::cout << "Movimientos: "
              << resultado.movimientos << "\n";

    std::cout << "Capturas: "
              << resultado.capturas << "\n";

    switch (resultado.estado) {
        case PARTIDA_GANA_PUMA:
            std::cout << "Ganador: PUMA\n";
            break;

        case PARTIDA_GANAN_PERROS:
            std::cout << "Ganador: PERROS\n";
            break;

        case PARTIDA_SIN_GANADOR:
            std::cout << "Partida terminada sin ganador\n";
            break;

        default:
            std::cout << "ERROR EN LA PARTIDA\n";
            break;
    }

    if (resultado.hilosCreados == 13 &&
        resultado.estado != PARTIDA_ERROR) {

        std::cout << "\nPRUEBA DEL JUEZ APROBADA\n";
        return 0;
    }

    std::cout << "\nPRUEBA DEL JUEZ FALLIDA\n";
    return 1;
}
