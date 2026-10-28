
#include "simulacion.h"

#include <pthread.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>

namespace {

constexpr int TOTAL_POSICIONES = 28;
constexpr int TOTAL_FICHAS = 13;

Posicion convertir(int id) {
    if (id < 25) return {id / 5, id % 5};
    if (id == 25) return {-1, 1};
    if (id == 26) return {-2, 2};
    return {-1, 3};
}

Casilla leerSinLock(Tablero& tablero, int id) {
    if (id < 25) {
        return tablero.celdas[id / 5][id % 5];
    }
    return tablero.trono[id - 25];
}

struct Contexto {
    Tablero* tablero;
    pthread_mutex_t mutexSimulacion;

    std::atomic<bool> detener{false};

    int posiciones[TOTAL_FICHAS];
    int movimientos = 0;
    int ganador = 0;

    int retardoMin;
    int retardoMax;
    unsigned int semilla;
};

struct DatosHilo {
    Contexto* contexto;
    int id;
};

void comprobarVictoria(Contexto& c) {
    if (victoriaPuma(*c.tablero)) {
        c.ganador = 1;
        c.detener.store(true);
    } else if (victoriaPerros(*c.tablero)) {
        c.ganador = 2;
        c.detener.store(true);
    }
}

void* ejecutarFicha(void* argumento) {
    auto* datos = static_cast<DatosHilo*>(argumento);
    Contexto& c = *datos->contexto;
    const int id = datos->id;

    std::mt19937 generador(
        c.semilla + static_cast<unsigned int>(id + 1) * 7919u
    );

    std::uniform_int_distribution<int> retardo(
        c.retardoMin, c.retardoMax
    );

    std::vector<int> destinos(TOTAL_POSICIONES);
    std::iota(destinos.begin(), destinos.end(), 0);

    while (!c.detener.load()) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(retardo(generador))
        );

        pthread_mutex_lock(&c.mutexSimulacion);

        if (c.detener.load() || c.posiciones[id] == -1) {
            pthread_mutex_unlock(&c.mutexSimulacion);
            break;
        }

        int origen = c.posiciones[id];

        std::shuffle(
            destinos.begin(), destinos.end(), generador
        );

        for (int destino : destinos) {
            Posicion desde = convertir(origen);
            Posicion hasta = convertir(destino);

            bool exito = false;

            if (id == 0) {
                exito = capturarPerro(*c.tablero, desde, hasta);

                if (exito) {
                    
                    Posicion medio = {
                        (desde.fila + hasta.fila) / 2,
                        (desde.columna + hasta.columna) / 2
                    };

                    for (int perro = 1; perro < TOTAL_FICHAS; perro++) {
                        if (c.posiciones[perro] == -1) continue;

                        Posicion p = convertir(c.posiciones[perro]);

                        if (p.fila == medio.fila &&
                            p.columna == medio.columna) {
                            c.posiciones[perro] = -1;
                            break;
                        }
                    }
                }
            }

            if (!exito) {
                exito = moverFicha(*c.tablero, desde, hasta);
            }

            if (exito) {
                c.posiciones[id] = destino;
                c.movimientos++;
                comprobarVictoria(c);
                break;
            }
        }

        pthread_mutex_unlock(&c.mutexSimulacion);
    }

    return nullptr;
}

} // namespace

ResultadoSimulacion simularPartida(
    Tablero& tablero,
    const ConfigSimulacion& config
) {
    ResultadoSimulacion resultado;

    if (config.retardoMinMs < 1 ||
        config.retardoMaxMs < config.retardoMinMs ||
        config.duracionMaxMs < 1) {
        std::cerr << "Configuracion invalida\n";
        return resultado;
    }

    Contexto contexto{};
    contexto.tablero = &tablero;
    contexto.retardoMin = config.retardoMinMs;
    contexto.retardoMax = config.retardoMaxMs;
    contexto.semilla = config.semilla;

    std::fill(
        std::begin(contexto.posiciones),
        std::end(contexto.posiciones),
        -1
    );

    pthread_mutex_init(&contexto.mutexSimulacion, nullptr);

    int pumas = 0;
    int perros = 0;

    pthread_mutex_lock(&tablero.mutex);

    for (int i = 0; i < TOTAL_POSICIONES; i++) {
        Casilla ficha = leerSinLock(tablero, i);

        if (ficha == PUMA) {
            pumas++;
            contexto.posiciones[0] = i;
        } else if (ficha == PERRO) {
            perros++;
            if (perros <= 12) {
                contexto.posiciones[perros] = i;
            }
        }
    }

    pthread_mutex_unlock(&tablero.mutex);

    if (pumas != 1 || perros != 12) {
        std::cerr << "Se requiere 1 Puma y 12 Perros\n";
        pthread_mutex_destroy(&contexto.mutexSimulacion);
        return resultado;
    }

    pthread_t hilos[TOTAL_FICHAS];
    DatosHilo datos[TOTAL_FICHAS];

    comprobarVictoria(contexto);

    if (!contexto.detener.load()) {
        for (int i = 0; i < TOTAL_FICHAS; i++) {
            datos[i] = {&contexto, i};

            int error = pthread_create(
                &hilos[i],
                nullptr,
                ejecutarFicha,
                &datos[i]
            );

            if (error != 0) {
                std::cerr << "Error al crear hilo " << i << "\n";
                contexto.detener.store(true);
                break;
            }

            resultado.hilosCreados++;
        }

        auto inicio = std::chrono::steady_clock::now();

        while (!contexto.detener.load()) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(10)
            );

            auto transcurrido =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - inicio
                ).count();

            if (transcurrido >= config.duracionMaxMs) {
                contexto.detener.store(true);
            }
        }
    }

    for (int i = 0; i < resultado.hilosCreados; i++) {
        pthread_join(hilos[i], nullptr);
    }

    resultado.movimientos = contexto.movimientos;
    resultado.ganador = contexto.ganador;

    pthread_mutex_lock(&tablero.mutex);
    resultado.capturas = tablero.perrosCapturados;
    pthread_mutex_unlock(&tablero.mutex);

    pthread_mutex_destroy(&contexto.mutexSimulacion);

    if (config.imprimirTableroFinal) {
        mostrarTablero(tablero);
    }

    return resultado;
}
