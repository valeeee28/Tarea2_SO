
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

void Matchmaker::iniciar() {
    {
        std::lock_guard<std::mutex> bloqueo(mutex_);

        if (iniciado_ || detenido_) {
            return;
        }

        iniciado_ = true;
    }

    for (int i = 0; i < maxMesas_; ++i) {
        hilosMesas_.emplace_back(
            &Matchmaker::ejecutarPartida, this, i + 1
        );
    }

    hiloGestor_ = std::thread(
        &Matchmaker::gestionarJugadores, this
    );
}

void Matchmaker::esperarFinalizacion() {
    if (hiloGestor_.joinable()) {
        hiloGestor_.join();
    }
}


void Matchmaker::detener() {
    {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        detenido_ = true;
    }

    cv_.notify_all();

    if (hiloGestor_.joinable()) {
        hiloGestor_.join();
    }

    for (std::thread& hilo : hilosMesas_) {
        if (hilo.joinable()) {
            hilo.join();
        }
    }

    hilosMesas_.clear();
}

void Matchmaker::gestionarJugadores() {
    int idJugador = 1;
    int idMesa = 1;

    while (true) {
        std::unique_lock<std::mutex> bloqueo(mutex_);

        if (detenido_ || jugadoresProcesados_ >= totalJugadores_) {
            break;
        }

        if (totalJugadores_ - jugadoresProcesados_ < 2) {
            std::cout << "Queda un jugador sin pareja.\n";
            jugadoresProcesados_ = totalJugadores_;
            break;
        }

        cv_.wait(bloqueo, [this] {
            return detenido_ || colaPartidas_.size() <
                                   static_cast<std::size_t>(maxMesas_);
        });

        if (detenido_) {
            break;
        }

        Partida partida{
            idMesa++,
            {idJugador++, RolJugador::PUMA},
            {idJugador++, RolJugador::PERROS}
        };

        jugadoresProcesados_ += 2;

        std::cout << "Partida " << partida.idMesa
                  << ": Puma " << partida.puma.id
                  << " vs. equipo de Perros "
                  << partida.perros.id << '\n';

        colaPartidas_.push(partida);

        bloqueo.unlock();
        cv_.notify_all();
    }

    cv_.notify_all();
}

void Matchmaker::ejecutarPartida(int idMesa) {
    while (true) {
        Partida partida{};

        {
            std::unique_lock<std::mutex> bloqueo(mutex_);

            
cv_.wait(bloqueo, [this, idMesa] {
    if (detenido_) {
        return true;
    }

    for (const Partida& pendiente : colaPartidas_) {
        if (pendiente.idMesa == idMesa) {
            return true;
        }
    }

    return false;
});

if (detenido_) {
    return;
}

auto it = std::find_if(
    colaPartidas_.begin(),
    colaPartidas_.end(),
    [idMesa](const Partida& pendiente) {
        return pendiente.idMesa == idMesa;
    }
);

if (it == colaPartidas_.end()) {
    continue;
}

partida = *it;
colaPartidas_.erase(it);

        }

        cv_.notify_all();

        std::cout << "Mesa " << idMesa
                  << ": iniciando partida "
                  << partida.idMesa << ".\n";

        // TODO: integrar aquí el juez y la lógica del tablero.

        std::cout << "Mesa " << idMesa
                  << ": finalizada partida "
                  << partida.idMesa << ".\n";
    }
}
