#ifndef MATCHMAKER_H
#define MATCHMAKER_H
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include "config.h"
#include <memory>
#include <atomic>
enum class RolJugador {
    PUMA,
    PERROS
};

struct Jugador {
    int id;
    RolJugador rol;
};

struct Partida {
    int idMesa;
    Jugador puma;
    Jugador perros;
};

class Matchmaker {
public:
Matchmaker(int totalJugadores, int maxMesas,
           const Config& config = Config{});
    ~Matchmaker();

    Matchmaker(const Matchmaker&) = delete;
    Matchmaker& operator=(const Matchmaker&) = delete;

    void iniciar();
    void esperarFinalizacion();
    void solicitarDetencion();
    void detener();
    bool abandonarJugador(int idMesa, RolJugador rol);

private:
    void gestionarJugadores();
    void ejecutarPartida(int idMesa);

    int totalJugadores_;
    int maxMesas_;

    int jugadoresProcesados_ = 0;
    bool detenido_ = false;
    bool iniciado_ = false;
    bool gestorTerminado_ = false;

    Config config_;
    std::atomic<bool> detenerSolicitado_{false};
    std::vector<std::shared_ptr<std::atomic<int>>> abandonoPorMesa_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::mutex mutexSalida_;
    std::vector<Partida> colaPartidas_;
    std::vector<std::thread> hilosMesas_;
    std::thread hiloGestor_;
};

#endif
