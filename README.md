# Tarea 2 – Sistemas Operativos
## Komikan Concurrente

### 1. Descripción del proyecto

El proyecto consiste en la implementación de un sistema concurrente para simular partidas del juego Komikan. El programa permite administrar múltiples partidas simultáneamente mediante hilos, gestionar jugadores y mesas, controlar el desarrollo de las partidas y manejar situaciones como el abandono de jugadores y la interrupción del torneo.

### 2. Estructura del proyecto

El proyecto se organiza en los siguientes módulos:

- `main.cpp`: punto de entrada del programa y administración del torneo.
- `src/matchmaker.h` y `src/matchmaker.cpp`: gestión de jugadores, asignación de partidas, mesas y coordinación del torneo.
- `src/config.h` y `config.cpp`: configuración de las partidas y posiciones iniciales.
- `board.h` y `board.cpp`: representación del tablero, movimientos y capturas.
- `juez.h` y `juez.cpp`: control y validación del desarrollo de las partidas.
- `simulacion.h` y `simulacion.cpp`: ejecución de la simulación concurrente.

### 3. Requisitos

-Windows 10/11 con Ubuntu mediante WSL, o Linux.

-Ubuntu instalado (en caso de utilizar WSL)

-Visual Studio Code.

-Extensión WSL de Visual Studio Code (si se utiliza WSL).

-Compilador G++.

-Soporte para C++17.

-Biblioteca de hilos POSIX (pthread).

### 4. Compilación

Desde la carpeta raíz del repositorio, ejecutar:

```bash
g++ -Wall -Wextra -std=c++17 -pthread src/main.cpp src/matchmaker.cpp config.cpp juez.cpp simulacion.cpp board.cpp -o torneo
```

### 5. Ejecución
Para iniciar el torneo se debe ejecutar el programa indicando dos parámetros obligatorios:
./torneo 4 2
El programa recibe como parámetros la cantidad de jugadores y la cantidad máxima de mesas.

Primer parámetro (4): cantidad total de jugadores que participarán en el torneo. En este caso, participan 4 jugadores.

Segundo parámetro (2): cantidad máxima de mesas disponibles para ejecutar partidas simultáneamente. En este caso, se pueden desarrollar hasta 2 partidas al mismo tiempo.

Ejemplo:

Al ejecutar ./torneo 4 2, el sistema recibe 4 jugadores y los organiza para disputar partidas utilizando un máximo de 2 mesas. Cada mesa puede ejecutar una partida de manera concurrente.

También se pueden utilizar otras cantidades de jugadores y mesas. Por ejemplo:

./torneo 8 2

En este caso, participan 8 jugadores y se permite un máximo de 2 partidas simultáneas.

Ejecución con configuración personalizada

Se puede indicar un tercer parámetro opcional para utilizar un archivo de configuración específico:

./torneo 4 2 config_personalizada.txt

Este archivo permite establecer parámetros de simulación, como los retardos, la semilla, la duración máxima y las posiciones iniciales de las fichas.

### 6. Concurrencia y control del torneo

El sistema utiliza hilos para ejecutar partidas de manera concurrente y administrar los movimientos de los jugadores.

Se implementan mecanismos de sincronización para proteger las operaciones sobre el tablero y evitar inconsistencias durante los movimientos y capturas.

Además, se contempla:

- Administración de jugadores y asignación de mesas.
- Límite de partidas simultáneas.
- Abandono de jugadores durante una partida.
- Detención ordenada del torneo mediante SIGINT (Ctrl+C).
- Priorización de la interrupción del torneo frente a solicitudes de abandono.

### 7. Pruebas realizadas

Se realizaron pruebas de funcionamiento, concurrencia e integridad:

| Prueba | Resultado |
|---|---|
| Ejecución normal | Aprobada |
| Configuración personalizada | Aprobada |
| Detención mediante SIGINT | Aprobada |
| Abandono de jugadores | 4/4 aprobadas |
| Prueba de estrés | 30/30 partidas aprobadas |
| Prueba de integridad | 4/4 aprobadas |
| Prueba del trono | 9/9 aprobadas |
| Configuración inválida | Rechazada correctamente |

### 8. Conclusión

La implementación permite ejecutar partidas de Komikan de forma concurrente, administrar los recursos disponibles y controlar correctamente la finalización de las partidas y del torneo. Las pruebas realizadas permitieron verificar el funcionamiento de los módulos, la integridad del tablero y el comportamiento del sistema ante situaciones de concurrencia, abandono e interrupción.
