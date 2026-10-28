
#include "simulacion.h"
#include <iostream>

int main() {
    Tablero tablero;
    inicializarTablero(tablero);

    colocarFicha(tablero, {2, 2}, PUMA);

    for (int i = 0; i < 5; i++) {
        colocarFicha(tablero, {0, i}, PERRO);
        colocarFicha(tablero, {4, i}, PERRO);
    }

    colocarFicha(tablero, {1, 0}, PERRO);
    colocarFicha(tablero, {3, 4}, PERRO);

    ConfigSimulacion config;
    config.semilla = 42;
    config.retardoMinMs = 1;
    config.retardoMaxMs = 5;
    config.duracionMaxMs = 1000;

    std::cout << "\nTABLERO INICIAL\n";
    mostrarTablero(tablero);

    ResultadoSimulacion resultado =
        simularPartida(tablero, config);

    std::cout << "\n--- RESULTADOS ---\n";
    std::cout << "Hilos creados: "
              << resultado.hilosCreados << "\n";
    std::cout << "Movimientos: "
              << resultado.movimientos << "\n";
    std::cout << "Capturas: "
              << resultado.capturas << "\n";

    if (resultado.ganador == 1) {
        std::cout << "Ganador: PUMA\n";
    } else if (resultado.ganador == 2) {
        std::cout << "Ganador: PERROS\n";
    } else {
        std::cout << "Ganador: SIN GANADOR\n";
    }

    int pumas = 0;
    int perros = 0;

    pthread_mutex_lock(&tablero.mutex);

    for (int f = 0; f < FILAS; f++) {
        for (int c = 0; c < COLUMNAS; c++) {
            if (tablero.celdas[f][c] == PUMA) pumas++;
            if (tablero.celdas[f][c] == PERRO) perros++;
        }
    }

    for (Casilla ficha : tablero.trono) {
        if (ficha == PUMA) pumas++;
        if (ficha == PERRO) perros++;
    }

    pthread_mutex_unlock(&tablero.mutex);

    bool correcto =
        resultado.hilosCreados == 13 &&
        pumas == 1 &&
        perros + resultado.capturas == 12 &&
        resultado.capturas >= 0 &&
        resultado.capturas <= 12;

    std::cout << "\n--- VALIDACION ---\n";
    std::cout << "Pumas en tablero: " << pumas << "\n";
    std::cout << "Perros restantes: " << perros << "\n";
    std::cout << "Perros capturados: "
              << resultado.capturas << "\n";

    std::cout << (correcto
        ? "PRUEBA DE CONSISTENCIA APROBADA\n"
        : "ERROR DE CONSISTENCIA\n");

    destruirTablero(tablero);

    return correcto ? 0 : 1;
}
