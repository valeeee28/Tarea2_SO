
#include "config.h"

#include <fstream>
#include <iostream>
#include <string>
#include <set>
#include <utility>

bool cargarConfiguracion(const std::string& ruta, Config& config) {
    std::ifstream archivo(ruta);

    if (!archivo.is_open()) {
        std::cerr << "No se pudo abrir el archivo de configuraciÃ³n: "
                  << ruta << '\n';
        return false;
    }

    std::string clave;
    int cantidadPumas = 0;
    int cantidadPerros = 0;
    auto valida = [](Posicion p) {
        return posicionValida(p.fila, p.columna);
    };
    std::set<std::pair<int,int>> ocupadas;

    while (archivo >> clave) {
        if (clave == "PUMA" || clave == "PERRO") {
            Posicion posicion{};
            if (!(archivo >> posicion.fila >> posicion.columna) ||
                !valida(posicion) ||
                !ocupadas.insert({posicion.fila, posicion.columna}).second) {
                std::cerr << "Posicion inicial invalida o repetida\n";
                return false;
            }
            if (clave == "PUMA") {
                if (++cantidadPumas > 1) return false;
                config.pumaInicial = posicion;
            } else {
                if (cantidadPerros >= 12) return false;
                config.perrosIniciales[cantidadPerros++] = posicion;
            }
        } else if (clave == "DELAY_MIN") {
            if (!(archivo >> config.delayMinMs)) {
                std::cerr << "Valor invÃ¡lido para DELAY_MIN\n";
                return false;
            }
        } else if (clave == "DELAY_MAX") {
            if (!(archivo >> config.delayMaxMs)) {
                std::cerr << "Valor invÃ¡lido para DELAY_MAX\n";
                return false;
            }
        } else if (clave == "SEED") {
            if (!(archivo >> config.seed)) {
                std::cerr << "Valor invÃ¡lido para SEED\n";
                return false;
            }
        } else if (clave == "DURACION_MAX") {
            if (!(archivo >> config.duracionMaxMs)) {
                std::cerr << "Valor invÃ¡lido para DURACION_MAX\n";
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

    if (cantidadPumas || cantidadPerros) {
        if (cantidadPumas != 1 || cantidadPerros != 12) {
            std::cerr << "Se requiere 1 Puma y 12 Perros\n";
            return false;
        }
        config.posicionesPersonalizadas = true;
    }

    if (config.delayMinMs < 1 ||
        config.delayMaxMs < config.delayMinMs) {
        std::cerr << "Los retardos de configuraciÃ³n no son vÃ¡lidos\n";
        return false;
    }

    if (config.duracionMaxMs < 1) {
        std::cerr << "DURACION_MAX debe ser mayor que cero\n";
        return false;
    }

    return true;
}
