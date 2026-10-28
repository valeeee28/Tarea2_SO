
#include "juez.h"

#include <pthread.h>
#include <semaphore.h>

#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

struct Servidor {
    sem_t mesasDisponibles;
    std::mutex mutexLog;

    std::atomic<int> mesasActivas{0};
    std::atomic<int> maximoObservado{0};
    std::atomic<int> partidasFinalizadas{0};
    std::atomic<int> partidasConError{0};

    ConfigSimulacion config;
};

struct DatosPartida {
    Servidor* servidor;
    int idPartida;
};

bool leerEnteroPositivo(const char* texto, int& valor) {
    try {
        std::string s(texto);
        size_t usados = 0;
        long numero = std::stol(s, &usados);

        if (usados != s.size() || numero <= 0 ||
            numero > 100000) {
            return false;
        }

        valor = static_cast<int>(numero);
        return true;
    } catch (...) {
        return false;
    }
}

bool cargarConfiguracion(
    const std::string& archivo,
    ConfigSimulacion& config
) {
    std::ifstream entrada(archivo);

    if (!entrada) {
        return false;
    }

    std::string clave;
    long long valor;

    while (entrada >> clave >> valor) {
        if (clave == "SEED" && valor >= 0) {
            config.semilla =
                static_cast<unsigned int>(valor);
        } else if (clave == "DELAY_MIN" && valor > 0) {
            config.retardoMinMs =
                static_cast<int>(valor);
        } else if (clave == "DELAY_MAX" && valor > 0) {
            config.retardoMaxMs =
                static_cast<int>(valor);
        } else if (clave == "DURACION_MAX" && valor > 0) {
            config.duracionMaxMs =
                static_cast<int>(valor);
        } else {
            return false;
        }
    }

    return entrada.eof() &&
           config.retardoMinMs > 0 &&
           config.retardoMaxMs >= config.retardoMinMs &&
           config.duracionMaxMs > 0;
}

void* ejecutarPartida(void* argumento) {
    auto* datos = static_cast<DatosPartida*>(argumento);
    Servidor& servidor = *datos->servidor;

    
    int espera;
    do {
        espera = sem_wait(&servidor.mesasDisponibles);
    } while (espera == -1 && errno == EINTR);

    if (espera == -1) {
        servidor.partidasConError++;
        return nullptr;
    }

    int activas = ++servidor.mesasActivas;

    int maximo = servidor.maximoObservado.load();
    while (activas > maximo &&
           !servidor.maximoObservado.compare_exchange_weak(
               maximo, activas)) {
    }

    {
        std::lock_guard<std::mutex> lock(
            servidor.mutexLog
        );

        std::cout << "[PARTIDA " << datos->idPartida
                  << "] Mesa asignada. Activas: "
                  << activas << "\n";
    }

    ConfigSimulacion config = servidor.config;
    config.semilla +=
        static_cast<unsigned int>(datos->idPartida);

    ResultadoJuez resultado = ejecutarJuez(
        datos->idPartida,
        config
    );

    if (resultado.estado == PARTIDA_ERROR) {
        servidor.partidasConError++;
    } else {
        servidor.partidasFinalizadas++;
    }

    activas = --servidor.mesasActivas;

    {
        std::lock_guard<std::mutex> lock(
            servidor.mutexLog
        );

        std::cout << "[PARTIDA " << datos->idPartida
                  << "] Mesa liberada. Activas: "
                  << activas << "\n";
    }

    sem_post(&servidor.mesasDisponibles);
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr
            << "Uso: ./torneo N M [config.txt]\n";
        return 1;
    }

    int jugadores;
    int maxMesas;

    if (!leerEnteroPositivo(argv[1], jugadores) ||
        !leerEnteroPositivo(argv[2], maxMesas)) {
        std::cerr << "N y M deben ser enteros positivos\n";
        return 1;
    }

    if (jugadores % 2 != 0) {
        std::cerr
            << "Esta version requiere N par: "
            << "cada partida utiliza 2 jugadores\n";
        return 1;
    }

    Servidor servidor;
    servidor.config.imprimirTableroFinal = false;

    if (argc == 4 &&
        !cargarConfiguracion(
            argv[3], servidor.config
        )) {
        std::cerr
            << "Error leyendo configuracion\n";
        return 1;
    }

    if (sem_init(
        &servidor.mesasDisponibles,
        0,
        static_cast<unsigned int>(maxMesas)
    ) != 0) {
        std::cerr << "Error inicializando semaforo\n";
        return 1;
    }

    int totalPartidas = jugadores / 2;

    std::vector<pthread_t> hilos(totalPartidas);
    std::vector<DatosPartida> datos(totalPartidas);

    int creados = 0;

    std::cout << "\n=== TORNEO KOMIKAN ===\n";
    std::cout << "Jugadores: " << jugadores << "\n";
    std::cout << "Mesas maximas: " << maxMesas << "\n";
    std::cout << "Partidas: " << totalPartidas << "\n";

    for (int i = 0; i < totalPartidas; i++) {
        datos[i] = {&servidor, i + 1};

        if (pthread_create(
            &hilos[i],
            nullptr,
            ejecutarPartida,
            &datos[i]
        ) != 0) {
            std::cerr << "Error creando partida\n";
            break;
        }

        creados++;
    }

    for (int i = 0; i < creados; i++) {
        pthread_join(hilos[i], nullptr);
    }

    sem_destroy(&servidor.mesasDisponibles);

    std::cout << "\n=== RESUMEN FINAL ===\n";
    std::cout << "Partidas finalizadas: "
              << servidor.partidasFinalizadas << "\n";
    std::cout << "Partidas con error: "
              << servidor.partidasConError << "\n";
    std::cout << "Maximo de mesas simultaneas: "
              << servidor.maximoObservado << "\n";
    std::cout << "Mesas activas al terminar: "
              << servidor.mesasActivas << "\n";

    bool correcto =
        creados == totalPartidas &&
        servidor.partidasConError == 0 &&
        servidor.partidasFinalizadas == totalPartidas &&
        servidor.maximoObservado <= maxMesas &&
        servidor.mesasActivas == 0;

    std::cout << (correcto
        ? "PRUEBA DE MESAS APROBADA\n"
        : "PRUEBA DE MESAS FALLIDA\n");

    return correcto ? 0 : 1;
}
