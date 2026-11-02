
#include "board.h"
#include <pthread.h>
#include <iostream>

struct Intento {
    Tablero* tablero;
    Posicion origen;
    Posicion destino;
    Casilla ficha;
    bool resultado = false;
};

void* intentarMover(void* dato) {
    auto* intento = static_cast<Intento*>(dato);

    intento->resultado = moverFicha(
        *intento->tablero,
        intento->origen,
        intento->destino,
        intento->ficha
    );

    return nullptr;
}

int main() {
    int aprobadas = 0;
    int total = 0;

    auto comprobar = [&](bool condicion, const char* nombre) {
        total++;
        std::cout << nombre << ": "
                  << (condicion ? "APROBADA" : "FALLIDA")
                  << "\n";
        if (condicion) aprobadas++;
    };

    Tablero t;
    inicializarTablero(t);

    colocarFicha(t, {2, 2}, PUMA);
    colocarFicha(t, {2, 3}, PERRO);

    Posicion capturado = {-99, -99};

    bool captura = capturarPerro(
        t, {2, 2}, {2, 4}, &capturado
    );

    comprobar(
        captura &&
        capturado.fila == 2 &&
        capturado.columna == 3 &&
        t.perrosCapturados == 1,
        "Captura con identificacion exacta"
    );

    comprobar(
        !moverFicha(t, {2, 4}, {3, 4}, PERRO),
        "Un hilo Perro no mueve al Puma"
    );

    comprobar(
        moverFicha(t, {2, 4}, {3, 4}, PUMA),
        "El hilo Puma mueve al Puma"
    );

    colocarFicha(t, {0, 0}, PERRO);
    colocarFicha(t, {0, 2}, PERRO);

    Intento a{&t, {0, 0}, {0, 1}, PERRO};
    Intento b{&t, {0, 2}, {0, 1}, PERRO};

    pthread_t hiloA;
    pthread_t hiloB;

    bool creadoA = pthread_create(
        &hiloA, nullptr, intentarMover, &a
    ) == 0;

    bool creadoB = pthread_create(
        &hiloB, nullptr, intentarMover, &b
    ) == 0;

    if (creadoA) pthread_join(hiloA, nullptr);
    if (creadoB) pthread_join(hiloB, nullptr);

    comprobar(
        creadoA && creadoB &&
        (a.resultado != b.resultado),
        "Dos hilos compiten por una casilla"
    );

    destruirTablero(t);

    std::cout << "\nRESULTADO: "
              << aprobadas << "/" << total
              << " PRUEBAS APROBADAS\n";

    return aprobadas == total ? 0 : 1;
}
