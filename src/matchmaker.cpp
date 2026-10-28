#include "../juez.h"
#include "matchmaker.h"
#include<algorithm>
#include <iostream>
#include <stdexcept>

Matchmaker::Matchmaker(int totalJugadores, int maxMesas, const Config& config)
    : totalJugadores_(totalJugadores),
      maxMesas_(maxMesas),
      config_(config) {
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

    cv_.notify_all();

    for (std::thread& hilo : hilosMesas_) {
        if (hilo.joinable()) {
            hilo.join();
        }
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

       int mesaAsignada = ((jugadoresProcesados_ / 2) % maxMesas_) + 1;

Partida partida{
    mesaAsignada,
    {idJugador++, RolJugador::PUMA},
    {idJugador++, RolJugador::PERROS}
};

        jugadoresProcesados_ += 2;

        std::cout << "Partida " << partida.idMesa
                  << ": Puma " << partida.puma.id
                  << " vs. equipo de Perros "
                  << partida.perros.id << '\n';

        colaPartidas_.push_back(partida);

        bloqueo.unlock();
        cv_.notify_all();
    }
    {
        std::lock_guard<std::mutex> bloqueo(mutex_);
        gestorTerminado_ = true;
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

                return gestorTerminado_;
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
                if (gestorTerminado_) {
                    return;
                }
                continue;
            }

            partida = *it;
            colaPartidas_.erase(it);
        }

        cv_.notify_all();

        ConfigSimulacion configJuez;
        configJuez.semilla = config_.seed;
        configJuez.retardoMinMs = config_.delayMinMs;
        configJuez.retardoMaxMs = config_.delayMaxMs;

        {
            std::lock_guard<std::mutex> bloqueoSalida(mutexSalida_);
            std::cout << "Mesa " << idMesa
                      << ": iniciando partida. Puma " << partida.puma.id
                      << " vs. Perros " << partida.perros.id << ".\n";
        }

        ResultadoJuez resultado = ejecutarJuez(idMesa, configJuez);

        {
            std::lock_guard<std::mutex> bloqueoSalida(mutexSalida_);
            std::cout << "Mesa " << idMesa
                      << ": partida finalizada. Estado: "
                      << resultado.estado
                      << ", movimientos: " << resultado.movimientos
                      << ", capturas: " << resultado.capturas << ".\n";
        }
    }
}

