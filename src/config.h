#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <array>
#include "../board.h"

struct Config {
    int duracionMaxMs = 5000;
    int delayMinMs = 50;
    int delayMaxMs = 300;
    unsigned int seed = 42;
    bool posicionesPersonalizadas = false;
    Posicion pumaInicial{2, 2};
    std::array<Posicion, 12> perrosIniciales{};
};

bool cargarConfiguracion(const std::string& ruta, Config& config);

#endif
