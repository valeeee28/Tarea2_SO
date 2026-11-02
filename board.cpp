
#include "board.h"
#include <iostream>
#include <algorithm>
#include <array>

namespace {

constexpr int N = 28;

int indice(Posicion p) {
    if (p.fila >= 0 && p.fila < FILAS &&
        p.columna >= 0 && p.columna < COLUMNAS) {
        return p.fila * COLUMNAS + p.columna;
    }

    if (p.fila == -1 && p.columna == 1)
        return TRONO_LEFT;

    if (p.fila == -2 && p.columna == 2)
        return TRONO_TOP;

    if (p.fila == -1 && p.columna == 3)
        return TRONO_RIGHT;

    return -1;
}

Posicion posicion(int id) {
    if (id < 25)
        return {id / COLUMNAS, id % COLUMNAS};

    if (id == TRONO_LEFT)
        return {-1, 1};

    if (id == TRONO_TOP)
        return {-2, 2};

    return {-1, 3};
}

Casilla& celda(Tablero& t, int id) {
    if (id < 25)
        return t.celdas[id / COLUMNAS][id % COLUMNAS];

    return t.trono[id - 25];
}

bool conectado(int a, int b) {
    if (a < 0 || b < 0 || a == b)
        return false;

    if (a < 25 && b < 25) {
        int af = a / 5;
        int ac = a % 5;
        int bf = b / 5;
        int bc = b % 5;

        return (af == bf &&
                (ac - bc == 1 || bc - ac == 1)) ||
               (ac == bc &&
                (af - bf == 1 || bf - af == 1));
    }

    const std::array<std::array<int, 2>, 7> conexiones = {{
        {26, 25},
        {26, 27},
        {26, 2},
        {25, 0},
        {25, 1},
        {27, 3},
        {27, 4}
    }};

    for (const auto& conexion : conexiones) {
        if ((a == conexion[0] && b == conexion[1]) ||
            (a == conexion[1] && b == conexion[0])) {
            return true;
        }
    }

    return false;
}

bool normalSinLock(Tablero& t, int a, int b) {
    return a >= 0 &&
           b >= 0 &&
           celda(t, a) != VACIA &&
           celda(t, b) == VACIA &&
           conectado(a, b);
}

bool saltoSinLock(
    Tablero& t,
    int a,
    int b,
    int& medio
) {
    if (a < 0 || b < 0 || a == b ||
        celda(t, a) != PUMA ||
        celda(t, b) != VACIA) {
        return false;
    }

    const Posicion origen = posicion(a);
    const Posicion destino = posicion(b);

    const int dx = destino.columna - origen.columna;
    const int dy = destino.fila - origen.fila;

    for (int m = 0; m < N; m++) {
        if (!conectado(a, m) ||
            !conectado(m, b) ||
            celda(t, m) != PERRO) {
            continue;
        }

        const Posicion centro = posicion(m);

        const int ux = centro.columna - origen.columna;
        const int uy = centro.fila - origen.fila;

        const int vx = destino.columna - centro.columna;
        const int vy = destino.fila - centro.fila;

        bool mismaLinea =
            ux * vy - uy * vx == 0;

        bool mismoSentido =
            ux * vx + uy * vy > 0;

        bool avanceValido =
            ux * dx + uy * dy > 0;

        if (mismaLinea &&
            mismoSentido &&
            avanceValido) {
            medio = m;
            return true;
        }
    }

    return false;
}

bool tieneMovimientosSinLock(Tablero& t) {
    for (int a = 0; a < N; a++) {
        if (celda(t, a) != PUMA)
            continue;

        for (int b = 0; b < N; b++) {
            if (normalSinLock(t, a, b))
                return true;

            int medio = -1;

            if (saltoSinLock(t, a, b, medio))
                return true;
        }
    }

    return false;
}

char simbolo(Casilla c) {
    if (c == PUMA)
        return 'P';

    if (c == PERRO)
        return 'D';

    return '.';
}

}

void inicializarTablero(Tablero& t) {
    pthread_mutex_init(&t.mutex, nullptr);

    pthread_mutex_lock(&t.mutex);

    for (auto& fila : t.celdas) {
        for (auto& c : fila) {
            c = VACIA;
        }
    }

    t.trono.assign(3, VACIA);
    t.perrosCapturados = 0;

    pthread_mutex_unlock(&t.mutex);
}

void mostrarTablero(Tablero& t) {
    pthread_mutex_lock(&t.mutex);

    std::cout << "\n--- TABLERO KOMIKAN ---\n";

    std::cout
        << "    "
        << simbolo(celda(t, 26))
        << "\n";

    std::cout
        << "  "
        << simbolo(celda(t, 25))
        << "   "
        << simbolo(celda(t, 27))
        << "\n";

    for (int f = 0; f < 5; f++) {
        for (int c = 0; c < 5; c++) {
            std::cout
                << simbolo(t.celdas[f][c])
                << ' ';
        }

        std::cout << '\n';
    }

    std::cout
        << "Perros capturados: "
        << t.perrosCapturados
        << '\n';

    pthread_mutex_unlock(&t.mutex);
}

void destruirTablero(Tablero& t) {
    pthread_mutex_destroy(&t.mutex);
}

bool posicionValida(int f, int c) {
    return indice({f, c}) >= 0;
}

bool colocarFicha(
    Tablero& t,
    Posicion p,
    Casilla ficha
) {
    int i = indice(p);

    if (i < 0)
        return false;

    pthread_mutex_lock(&t.mutex);

    celda(t, i) = ficha;

    pthread_mutex_unlock(&t.mutex);

    return true;
}

bool movimientoValido(
    Tablero& t,
    Posicion o,
    Posicion d
) {
    int a = indice(o);
    int b = indice(d);

    pthread_mutex_lock(&t.mutex);

    bool ok = normalSinLock(t, a, b);

    pthread_mutex_unlock(&t.mutex);

    return ok;
}

bool moverFicha(
    Tablero& t,
    Posicion o,
    Posicion d,
    Casilla fichaEsperada
) {
    int a = indice(o);
    int b = indice(d);

    pthread_mutex_lock(&t.mutex);

    bool ok =
        a >= 0 &&
        b >= 0 &&
        fichaEsperada != VACIA &&
        celda(t, a) == fichaEsperada &&
        normalSinLock(t, a, b);

    if (ok) {
        celda(t, b) = celda(t, a);
        celda(t, a) = VACIA;
    }

    pthread_mutex_unlock(&t.mutex);

    return ok;
}

bool moverFicha(
    Tablero& t,
    Posicion o,
    Posicion d
) {
    int a = indice(o);

    if (a < 0)
        return false;

    pthread_mutex_lock(&t.mutex);

    Casilla ficha = celda(t, a);

    pthread_mutex_unlock(&t.mutex);

    return moverFicha(t, o, d, ficha);
}

bool capturarPerro(
    Tablero& t,
    Posicion o,
    Posicion d,
    Posicion* perroCapturado
) {
    int a = indice(o);
    int b = indice(d);
    int m = -1;

    pthread_mutex_lock(&t.mutex);

    bool ok = saltoSinLock(t, a, b, m);

    if (ok) {
        if (perroCapturado != nullptr) {
            *perroCapturado = posicion(m);
        }

        celda(t, a) = VACIA;
        celda(t, m) = VACIA;
        celda(t, b) = PUMA;

        ++t.perrosCapturados;
    }

    pthread_mutex_unlock(&t.mutex);

    return ok;
}

bool capturarPerro(
    Tablero& t,
    Posicion o,
    Posicion d
) {
    return capturarPerro(
        t,
        o,
        d,
        nullptr
    );
}

bool victoriaPuma(Tablero& t) {
    pthread_mutex_lock(&t.mutex);

    bool ok = t.perrosCapturados >= 6;

    pthread_mutex_unlock(&t.mutex);

    return ok;
}

bool pumaTieneMovimientos(Tablero& t) {
    pthread_mutex_lock(&t.mutex);

    bool ok = tieneMovimientosSinLock(t);

    pthread_mutex_unlock(&t.mutex);

    return ok;
}

bool victoriaPerros(Tablero& t) {
    pthread_mutex_lock(&t.mutex);

    bool hayPuma = false;

    for (int i = 0; i < N; i++) {
        if (celda(t, i) == PUMA) {
            hayPuma = true;
            break;
        }
    }

    bool ok =
        hayPuma &&
        !tieneMovimientosSinLock(t);

    pthread_mutex_unlock(&t.mutex);

    return ok;
}
