#include "matchmaker.h"
#include "config.h"

#include <exception>
#include <iostream>
#include <string>

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

        Matchmaker servidor(totalJugadores, maxMesas,config);

        std::cout << "Servidor Komikan iniciado.\n";

        servidor.iniciar();
        servidor.esperarFinalizacion();
        servidor.detener();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}