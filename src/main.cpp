
#include "matchmaker.h"
#include "config.h"

#include <exception>
#include <iostream>
#include <string>
#include <csignal>
#include <pthread.h>
#include <thread>
#include <atomic>
#include <ctime>

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Uso: ./torneo N M [config.txt]\n";
        return 1;
    }

    try {
        int totalJugadores = std::stoi(argv[1]);
        int maxMesas = std::stoi(argv[2]);

        Config config;

        if (argc == 4) {
            if (!cargarConfiguracion(argv[3], config)) {
                return 1;
            }
        }

        // Bloquear SIGINT antes de crear los hilos.
        sigset_t conjuntoSenales;
        sigemptyset(&conjuntoSenales);
        sigaddset(&conjuntoSenales, SIGINT);

        int error = pthread_sigmask(
            SIG_BLOCK, &conjuntoSenales, nullptr
        );

        if (error != 0) {
            std::cerr << "Error al bloquear SIGINT.\n";
            return 1;
        }

        Matchmaker servidor(totalJugadores, maxMesas, config);

        std::atomic<bool> finalizarMonitor{false};

        std::cout << "Servidor Komikan iniciado.\n";

        servidor.iniciar();

        // Hilo encargado de esperar Ctrl+C.
        std::thread hiloSenales([&]() {
            while (!finalizarMonitor.load()) {
                struct timespec espera {
                    0, 100000000
                };

                int senal = sigtimedwait(
                    &conjuntoSenales,
                    nullptr,
                    &espera
                );

                if (senal == SIGINT) {
                    std::cout << "\nSIGINT recibido. "
                              << "Solicitando cierre ordenado...\n";

                    servidor.solicitarDetencion();
                    break;
                }
            }
        });

        // Esperar el término normal o la cancelación.
        servidor.esperarFinalizacion();

        finalizarMonitor.store(true);

        if (hiloSenales.joinable()) {
            hiloSenales.join();
        }

        servidor.detener();

        std::cout << "Torneo finalizado correctamente.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
