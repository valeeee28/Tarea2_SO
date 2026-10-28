
#include "matchmaker.h"

#include <iostream>
#include <stdexcept>

Matchmaker::Matchmaker(int totalJugadores, int maxMesas)
    : totalJugadores_(totalJugadores),
      maxMesas_(maxMesas) {
    if (totalJugadores < 0 || maxMesas <= 0) {
        throw std::invalid_argument(
            "La cantidad de jugadores no puede ser negativa "
            "y debe existir al menos una mesa."
        );
    }
}

Matchmaker::~Matchmaker() {
    detener();
}
