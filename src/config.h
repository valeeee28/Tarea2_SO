#ifndef CONFIG_H
#define CONFIG_H

#include <string>

struct Config {
    int duracionMaxMs = 5000;
    int delayMinMs = 50;
    int delayMaxMs = 300;
    unsigned int seed = 42;
};

bool cargarConfiguracion(const std::string& ruta, Config& config);

#endif
