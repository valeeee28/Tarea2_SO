
#include "simulacion.h"

#include <pthread.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <numeric>
#include <random>
#include <thread>
#include <vector>
#include <iostream>

namespace {

constexpr int TOTAL_FICHAS = 13;
constexpr int TOTAL_POSICIONES = 28;

using Reloj = std::chrono::steady_clock;

Posicion convertir(int id) {
    if (id < 0 || id >= TOTAL_POSICIONES) {
        return {-99, -99};
    }

    if (id < 25) {
        return {id / 5, id % 5};
    }

    if (id == 25) return {-1, 1};
    if (id == 26) return {-2, 2};

    return {-1, 3};
}

Casilla leerSinLock(Tablero& t, int id) {
    if (id < 25) {
        return t.celdas[id / 5][id % 5];
    }

    return t.trono[id - 25];
}

struct Contexto {
    Tablero* tablero = nullptr;

    const std::atomic<bool>* detenerExterno = nullptr;

    pthread_mutex_t mutexSimulacion{};

    std::atomic<bool> detener{false};

    std::array<int, TOTAL_FICHAS> posiciones{};

    int movimientos = 0;
    int ganador = 0;

    bool errorIntegridad = false;

    int retardoMin = 50;
    int retardoMax = 300;

    unsigned int semilla = 42;
    int imprimirCadaMovimientos = 0;
};

struct DatosHilo {
    Contexto* contexto;
    int id;
};

bool cancelacionExterna(const Contexto& c) {
    return c.detenerExterno != nullptr &&
           c.detenerExterno->load();
}

bool debeDetenerse(const Contexto& c) {
    return c.detener.load() || cancelacionExterna(c);
}

bool esperarInterrumpible(
    Contexto& c,
    int milisegundos
) {
    int transcurridos = 0;

    while (transcurridos < milisegundos) {
        if (debeDetenerse(c)) {
            return false;
        }

        int tramo = std::min(
            5,
            milisegundos - transcurridos
        );

        std::this_thread::sleep_for(
            std::chrono::milliseconds(tramo)
        );

        transcurridos += tramo;
    }

    return !debeDetenerse(c);
}

// Requiere mutexSimulacion bloqueado.
void comprobarVictoria(Contexto& c) {
    if (debeDetenerse(c)) {
        return;
    }

    if (victoriaPuma(*c.tablero)) {
        c.ganador = 1;
        c.detener.store(true);
    } else if (victoriaPerros(*c.tablero)) {
        c.ganador = 2;
        c.detener.store(true);
    }
}

// Requiere mutexSimulacion bloqueado.
bool registrarCaptura(
    Contexto& c,
    Posicion capturado
) {
    for (int perro = 1;
         perro < TOTAL_FICHAS;
         perro++) {

        if (c.posiciones[perro] == -1) {
            continue;
        }

        Posicion actual = convertir(
            c.posiciones[perro]
        );

        if (actual.fila == capturado.fila &&
            actual.columna == capturado.columna) {

            c.posiciones[perro] = -1;
            return true;
        }
    }

    return false;
}

void* ejecutarFicha(void* argumento) {
    auto* datos = static_cast<DatosHilo*>(argumento);

    Contexto& c = *datos->contexto;
    int id = datos->id;

    std::mt19937 rng(
        c.semilla +
        static_cast<unsigned int>(id + 1) * 7919u
    );

    std::uniform_int_distribution<int> retardo(
        c.retardoMin,
        c.retardoMax
    );

    std::vector<int> destinos(TOTAL_POSICIONES);

    std::iota(
        destinos.begin(),
        destinos.end(),
        0
    );

    while (!debeDetenerse(c)) {
        if (!esperarInterrumpible(c, retardo(rng))) {
            break;
        }

        pthread_mutex_lock(&c.mutexSimulacion);

        if (debeDetenerse(c) ||
            c.posiciones[id] == -1) {

            pthread_mutex_unlock(
                &c.mutexSimulacion
            );

            break;
        }

        int origen = c.posiciones[id];

        std::shuffle(
            destinos.begin(),
            destinos.end(),
            rng
        );

        for (int destino : destinos) {
            if (debeDetenerse(c)) {
                break;
            }

            Posicion desde = convertir(origen);
            Posicion hasta = convertir(destino);

            bool exito = false;

            if (id == 0) {
                Posicion capturado = {-99, -99};

                exito = capturarPerro(
                    *c.tablero,
                    desde,
                    hasta,
                    &capturado
                );

                if (exito) {
                    bool encontrado = registrarCaptura(
                        c,
                        capturado
                    );

                    if (!encontrado) {
                        std::cerr
                            << "Error: perro capturado "
                            << "sin hilo asociado\n";

                        c.errorIntegridad = true;
                        c.detener.store(true);
                        break;
                    }
                }
            }

            if (!exito) {
                Casilla fichaEsperada =
                    id == 0 ? PUMA : PERRO;

                exito = moverFicha(
                    *c.tablero,
                    desde,
                    hasta,
                    fichaEsperada
                );
            }

            if (exito) {
                c.posiciones[id] = destino;
                c.movimientos++;
                if (c.imprimirCadaMovimientos > 0 &&
                    c.movimientos % c.imprimirCadaMovimientos == 0) {
                    std::cout << "\nMovimiento " << c.movimientos << "\n";
                    mostrarTablero(*c.tablero);
                }

                comprobarVictoria(c);
                break;
            }
        }

        pthread_mutex_unlock(
            &c.mutexSimulacion
        );
    }

    return nullptr;
}

} // namespace

ResultadoSimulacion simularPartida(
    Tablero& tablero,
    const ConfigSimulacion& config,
    const std::atomic<bool>* detenerExterno
) {
    ResultadoSimulacion resultado;

    if (config.retardoMinMs < 1 ||
        config.retardoMaxMs < config.retardoMinMs ||
        config.duracionMaxMs < 1) {

        return resultado;
    }

    if (detenerExterno != nullptr &&
        detenerExterno->load()) {

        resultado.motivo = FIN_CANCELACION;
        return resultado;
    }

    Contexto c;

    c.tablero = &tablero;
    c.detenerExterno = detenerExterno;
    c.retardoMin = config.retardoMinMs;
    c.retardoMax = config.retardoMaxMs;
    c.semilla = config.semilla;
    c.imprimirCadaMovimientos = config.imprimirCadaMovimientos;

    c.posiciones.fill(-1);

    if (pthread_mutex_init(
        &c.mutexSimulacion,
        nullptr
    ) != 0) {

        return resultado;
    }

    int pumas = 0;
    int perros = 0;

    pthread_mutex_lock(&tablero.mutex);

    for (int i = 0;
         i < TOTAL_POSICIONES;
         i++) {

        Casilla ficha = leerSinLock(
            tablero,
            i
        );

        if (ficha == PUMA) {
            pumas++;
            c.posiciones[0] = i;
        } else if (ficha == PERRO) {
            perros++;

            if (perros <= 12) {
                c.posiciones[perros] = i;
            }
        }
    }

    pthread_mutex_unlock(&tablero.mutex);

    if (pumas != 1 || perros != 12) {
        pthread_mutex_destroy(
            &c.mutexSimulacion
        );

        return resultado;
    }

    pthread_t hilos[TOTAL_FICHAS];
    DatosHilo datos[TOTAL_FICHAS];

    bool errorCreacion = false;

    comprobarVictoria(c);

    for (int i = 0;
         i < TOTAL_FICHAS;
         i++) {

        if (debeDetenerse(c)) {
            break;
        }

        datos[i] = {&c, i};

        if (pthread_create(
            &hilos[i],
            nullptr,
            ejecutarFicha,
            &datos[i]
        ) != 0) {

            errorCreacion = true;
            c.detener.store(true);
            break;
        }

        resultado.hilosCreados++;
    }

    auto inicio = Reloj::now();

    bool tiempoAgotado = false;

    while (!debeDetenerse(c) &&
           !errorCreacion) {

        if (Reloj::now() - inicio >=
            std::chrono::milliseconds(
                config.duracionMaxMs
            )) {

            tiempoAgotado = true;
            c.detener.store(true);
            break;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(5)
        );
    }

    for (int i = 0;
         i < resultado.hilosCreados;
         i++) {

        pthread_join(
            hilos[i],
            nullptr
        );
    }

    resultado.movimientos = c.movimientos;
    resultado.ganador = c.ganador;

    pthread_mutex_lock(&tablero.mutex);

    resultado.capturas =
        tablero.perrosCapturados;

    pthread_mutex_unlock(&tablero.mutex);

    if (errorCreacion || c.errorIntegridad) {
        resultado.motivo = FIN_ERROR;
        resultado.ganador = 0;
    } else if (cancelacionExterna(c)) {
        resultado.motivo = FIN_CANCELACION;
        resultado.ganador = 0;
    } else if (c.ganador == 1) {
        resultado.motivo = FIN_VICTORIA_PUMA;
    } else if (c.ganador == 2) {
        resultado.motivo = FIN_VICTORIA_PERROS;
    } else if (tiempoAgotado) {
        resultado.motivo = FIN_TIEMPO;
    } else {
        resultado.motivo = FIN_ERROR;
    }

    pthread_mutex_destroy(
        &c.mutexSimulacion
    );

    if (config.imprimirTableroFinal) {
        mostrarTablero(tablero);
    }

    return resultado;
}
