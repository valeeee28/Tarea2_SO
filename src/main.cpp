
#include "matchmaker.h"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Uso: ./torneo N M [config.txt]\n";
        return 1;
    }

    try {
        int totalJugadores = std::stoi(argv[1]);
        int maxMesas = std::stoi(argv[2]);

        Matchmaker servidor(totalJugadores, maxMesas);
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
