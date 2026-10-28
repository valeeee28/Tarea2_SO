
#include "config.h"

#include <fstream>
#include <iostream>
#include <string>

bool cargarConfiguracion(const std::string& ruta, Config& config) {
    std::ifstream archivo(ruta);

    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de configuración: "
                  << ruta << '\n';
        return false;
    }

    std::string clave;

    while (archivo >> clave) {
        if (clave == "DELAY_MIN") {
            if (!(archivo >> config.delayMinMs)) {
                std::cerr << "Valor inválido para DELAY_MIN\n";
                return false;
            }
        } else if (clave == "DELAY_MAX") {
            if (!(archivo >> config.delayMaxMs)) {
                std::cerr << "Valor inválido para DELAY_MAX\n";
                return false;
            }
        } else if (clave == "SEED") {
            if (!(archivo >> config.seed)) {
                std::cerr << "Valor inválido para SEED\n";
                return false;
            }
        } else {
            std::string valorIgnorado;
            if (!(archivo >> valorIgnorado)) {
                std::cerr << "Falta un valor para: " << clave << '\n';
                return false;
            }
        }
    }

    if (config.delayMinMs < 0 ||
        config.delayMaxMs < config.delayMinMs) {
        std::cerr << "Los retardos de configuración no son válidos\n";
        return false;
    }

    return true;
}
