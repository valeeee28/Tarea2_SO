#include "board.h"
#include <pthread.h>
#include <cassert>
#include <iostream>

struct Intento { Tablero* tablero; Posicion origen,destino; bool resultado=false; };
void* hiloMover(void* arg) {
    auto* x=static_cast<Intento*>(arg);
    x->resultado=moverFicha(*x->tablero,x->origen,x->destino);
    return nullptr;
}

int main() {
    Tablero t; inicializarTablero(t);
    assert(colocarFicha(t,{2,2},PUMA));
    assert(colocarFicha(t,{2,3},PERRO));
    assert(!moverFicha(t,{2,2},{4,4}));
    assert(capturarPerro(t,{2,2},{2,4}));
    assert(t.perrosCapturados==1);
    assert(!victoriaPuma(t));
    mostrarTablero(t);
    destruirTablero(t);

    Tablero tr; inicializarTablero(tr);
    assert(colocarFicha(tr,{-2,2},PUMA));
    assert(moverFicha(tr,{-2,2},{-1,1}));
    assert(moverFicha(tr,{-1,1},{0,0}));
    assert(!moverFicha(tr,{0,0},{2,2}));
    mostrarTablero(tr);
    destruirTablero(tr);

    Tablero c; inicializarTablero(c);
    assert(colocarFicha(c,{1,2},PERRO));
    assert(colocarFicha(c,{2,1},PERRO));
    Intento a{&c,{1,2},{2,2}}, b{&c,{2,1},{2,2}};
    pthread_t h1,h2;
    assert(pthread_create(&h1,nullptr,hiloMover,&a)==0);
    assert(pthread_create(&h2,nullptr,hiloMover,&b)==0);
    pthread_join(h1,nullptr); pthread_join(h2,nullptr);
    assert(a.resultado != b.resultado);
    assert(c.celdas[2][2]==PERRO);
    mostrarTablero(c);
    destruirTablero(c);
    std::cout << "\nTODAS LAS PRUEBAS PASARON\n";
}
