
#include "board.h"
#include <iostream>

static int fallos = 0;

void comprobar(bool valor, const char* nombre) {
    std::cout
        << (valor ? "APROBADA: " : "FALLIDA: ")
        << nombre << '\n';

    if (!valor) {
        ++fallos;
    }
}

int main() {
    {
        Tablero t;
        inicializarTablero(t);

        colocarFicha(t, {-1, 1}, PUMA);

        comprobar(
            !moverFicha(t, {-1, 1}, {-1, 3}, PUMA),
            "No existe arista horizontal L-R"
        );

        comprobar(
            moverFicha(t, {-1, 1}, {-2, 2}, PUMA),
            "L a T permitido"
        );

        destruirTablero(t);
    }

    {
        Tablero t;
        inicializarTablero(t);

        colocarFicha(t, {-2, 2}, PUMA);
        colocarFicha(t, {-1, 1}, PERRO);

        Posicion perro{-99, -99};

        comprobar(
            capturarPerro(t, {-2, 2}, {0, 0}, &perro),
            "Captura diagonal T-L-esquina"
        );

        comprobar(
            perro.fila == -1 && perro.columna == 1,
            "Identifica perro diagonal"
        );

        comprobar(
            t.perrosCapturados == 1,
            "Cuenta captura diagonal"
        );

        destruirTablero(t);
    }

    {
        Tablero t;
        inicializarTablero(t);

        colocarFicha(t, {-2, 2}, PUMA);
        colocarFicha(t, {0, 2}, PERRO);

        Posicion perro{-99, -99};

        comprobar(
            capturarPerro(t, {-2, 2}, {1, 2}, &perro),
            "Captura vertical T-centro-fila1"
        );

        comprobar(
            perro.fila == 0 && perro.columna == 2,
            "Identifica perro vertical"
        );

        destruirTablero(t);
    }

    {
        Tablero t;
        inicializarTablero(t);

        colocarFicha(t, {-1, 1}, PUMA);
        colocarFicha(t, {0, 1}, PERRO);

        comprobar(
            capturarPerro(t, {-1, 1}, {1, 1}),
            "Captura vertical desde lateral"
        );

        destruirTablero(t);
    }

    {
        Tablero t;
        inicializarTablero(t);

        colocarFicha(t, {-2, 2}, PUMA);
        colocarFicha(t, {-1, 1}, PERRO);

        comprobar(
            !capturarPerro(t, {-2, 2}, {0, 1}),
            "No captura con cambio de direccion"
        );

        destruirTablero(t);
    }

    std::cout
        << "\nTRONO: "
        << (fallos ? "FALLIDO" : "TODAS APROBADAS")
        << '\n';

    return fallos ? 1 : 0;
}
