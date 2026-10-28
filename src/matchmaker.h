
#ifndef MATCHMAKER_H
#define MATCHMAKER_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class Matchmaker {
public:
    Matchmaker(int totalJugadores, int maxMesas);
    ~Matchmaker();

    Matchmaker(const Matchmaker&) = delete;
    Matchmaker& operator=(const Matchmaker&) = delete;

    void iniciar();
    void detener();

private:
    void gestionarJugadores();
    void ejecutarPartida(int idMesa);

    int totalJugadores_;
    int maxMesas_;

    int jugadoresProcesados_ = 0;
    int mesasActivas_ = 0;
    bool detenido_ = false;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<int> colaJugadores_;
    std::vector<std::thread> hilosMesas_;
};

#endif
