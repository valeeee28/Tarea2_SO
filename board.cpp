#include "board.h"
#include <iostream>
#include <algorithm>
#include <array>

namespace {
constexpr int N=28;

int indice(Posicion p) {
    if(p.fila>=0 && p.fila<FILAS && p.columna>=0 && p.columna<COLUMNAS)
        return p.fila*COLUMNAS+p.columna;
    if(p.fila==-1 && p.columna==1) return TRONO_LEFT;

    if(p.fila==-2 && p.columna==2) return TRONO_TOP;

    if(p.fila==-1 && p.columna==3) return TRONO_RIGHT;
    return -1;
}

Posicion posicion(int id) {
    if(id<25) return {id/COLUMNAS,id%COLUMNAS};

    if(id==TRONO_LEFT) return {-1,1};

    if(id==TRONO_TOP) return {-2,2};
    return {-1,3};
}
Casilla& celda(Tablero& t,int id) {
    if(id<25) return t.celdas[id/COLUMNAS][id%COLUMNAS];
    return t.trono[id-25];
}

bool conectado(int a,int b) {
    if(a<0||b<0||a==b) return false;

    if(a<25 && b<25) {
        int af=a/5,ac=a%5,bf=b/5,bc=b%5;
        return (af==bf && (ac-bc==1||bc-ac==1)) ||
               (ac==bc && (af-bf==1||bf-af==1));
    }

    const std::array<std::array<int,2>,8> e = {{{26,25},{26,27},{26,2},
       {25,27},{25,0},{25,1},{27,3},{27,4}}};
    for(const auto& x:e) if((a==x[0]&&b==x[1])||(a==x[1]&&b==x[0])) return true;
    return false;
}
bool normalSinLock(Tablero& t,int a,int b) {
    return a>=0 && b>=0 && celda(t,a)!=VACIA &&
           celda(t,b)==VACIA && conectado(a,b);
}

bool saltoSinLock(Tablero& t,int a,int b,int& medio) {
    if(a<0||b<0||celda(t,a)!=PUMA||celda(t,b)!=VACIA) return false;

    Posicion p=posicion(a),q=posicion(b);

    for(int m=0;m<N;m++) {
        Posicion r=posicion(m);
        if(conectado(a,m)&&conectado(m,b) &&
           2*r.fila==p.fila+q.fila &&
           2*r.columna==p.columna+q.columna &&
           celda(t,m)==PERRO) { medio=m; return true; }
    }
    return false;
}

bool tieneMovimientosSinLock(Tablero& t) {
    for(int a=0;a<N;a++) if(celda(t,a)==PUMA) {
        for(int b=0;b<N;b++) {
            if(normalSinLock(t,a,b)) return true;
            int medio=-1;
            if(saltoSinLock(t,a,b,medio)) return true;
        }
    }
    return false;
}
char simbolo(Casilla c) { return c==PUMA?'P':c==PERRO?'D':'.'; }
}

void inicializarTablero(Tablero& t) {
    pthread_mutex_init(&t.mutex,nullptr);
    pthread_mutex_lock(&t.mutex);
    for(auto& fila:t.celdas) for(auto& c:fila) c=VACIA;
    t.trono.assign(3,VACIA);
    t.perrosCapturados=0;
    pthread_mutex_unlock(&t.mutex);
}
void mostrarTablero(Tablero& t) {
    pthread_mutex_lock(&t.mutex);
    std::cout << "\n--- TABLERO KOMIKAN ---\n";
    std::cout << "    " << simbolo(celda(t,26)) << "\n";
    std::cout << "  " << simbolo(celda(t,25)) << "   " << simbolo(celda(t,27)) << "\n";
    for(int f=0;f<5;f++) {
        for(int c=0;c<5;c++) std::cout << simbolo(t.celdas[f][c]) << ' ';
        std::cout << '\n';
    }
    std::cout << "Perros capturados: " << t.perrosCapturados << '\n';
    pthread_mutex_unlock(&t.mutex);
}
void destruirTablero(Tablero& t) { pthread_mutex_destroy(&t.mutex); }
bool posicionValida(int f,int c) { return indice({f,c})>=0; }
bool colocarFicha(Tablero& t,Posicion p,Casilla ficha) {
    int i=indice(p); if(i<0) return false;
    pthread_mutex_lock(&t.mutex);
    celda(t,i)=ficha;
    pthread_mutex_unlock(&t.mutex);
    return true;
}
bool movimientoValido(Tablero& t,Posicion o,Posicion d) {
    int a=indice(o),b=indice(d);
    pthread_mutex_lock(&t.mutex);
    bool ok=normalSinLock(t,a,b);
    pthread_mutex_unlock(&t.mutex);
    return ok;
}
bool moverFicha(Tablero& t,Posicion o,Posicion d) {
    int a=indice(o),b=indice(d);
    pthread_mutex_lock(&t.mutex);
    bool ok=normalSinLock(t,a,b);
    if(ok) { celda(t,b)=celda(t,a); celda(t,a)=VACIA; }
    pthread_mutex_unlock(&t.mutex);
    return ok;
}
bool capturarPerro(Tablero& t,Posicion o,Posicion d) {
    int a=indice(o),b=indice(d),m=-1;
    pthread_mutex_lock(&t.mutex);
    bool ok=saltoSinLock(t,a,b,m);
    if(ok) { celda(t,a)=VACIA; celda(t,m)=VACIA; celda(t,b)=PUMA; ++t.perrosCapturados; }
    pthread_mutex_unlock(&t.mutex);
    return ok;
}
bool victoriaPuma(Tablero& t) {
    pthread_mutex_lock(&t.mutex);
    bool ok=t.perrosCapturados>=6;
    pthread_mutex_unlock(&t.mutex);
    return ok;
}
bool pumaTieneMovimientos(Tablero& t) {
    pthread_mutex_lock(&t.mutex);
    bool ok=tieneMovimientosSinLock(t);
    pthread_mutex_unlock(&t.mutex);
    return ok;
}
bool victoriaPerros(Tablero& t) {
    pthread_mutex_lock(&t.mutex);
    bool hayPuma=false;
    for(int i=0;i<N;i++) if(celda(t,i)==PUMA) hayPuma=true;
    bool ok=hayPuma && !tieneMovimientosSinLock(t);
    pthread_mutex_unlock(&t.mutex);
    return ok;
}
