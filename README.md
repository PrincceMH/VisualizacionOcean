# Visualización de océano

Escena interactiva en C++ con OpenGL/GLUT. La superficie del océano se anima
mediante una suma de ondas sinusoidales y se integra con una isla, un faro y un
bote que sigue la altura del agua.

## Funcionalidades

- Malla polar circular de `80` anillos, `200` sectores y radio `50`.
- Animación mediante *Sum of Sines* y espectro configurable.
- Espectro principal en `data/spectrum_realista.txt` con olas de distintas
  escalas y direcciones.
- Normales suavizadas por vértice y costura circular continua.
- Textura repetible del océano mediante `assets/textures/ocean.tga`.
- Espuma visual aditiva en las crestas.
- Niebla de horizonte aplicada únicamente a la geometría de la escena.
- Atardecer fijo con bóveda celeste y sol visual sincronizado con la luz
  direccional.
- Luz solar cálida (`GL_LIGHT0`) y luz fría de relleno del cielo (`GL_LIGHT2`).
- Bote con altura e inclinación longitudinal calculadas desde las mismas ondas
  del océano.
- Isla procedural y faro posicionados dentro de la escena.
- Cámara con posición controlada, orientación por `yaw`/`pitch` y zoom óptico.

## Compilación y ejecución

El programa debe compilarse y ejecutarse desde la raíz del repositorio para que
encuentre `data/` y `assets/`.

### Windows (MinGW + freeglut)

```bash
g++ src/main.cpp src/Ocean.cpp src/Wave.cpp src/Environment.cpp src/Boat.cpp src/Island.cpp src/Lighthouse.cpp -o Oceano.exe -lopengl32 -lglu32 -lfreeglut
./Oceano.exe
```

### macOS

```bash
g++ -std=c++11 src/main.cpp src/Ocean.cpp src/Wave.cpp src/Environment.cpp src/Boat.cpp src/Island.cpp src/Lighthouse.cpp -o Oceano -framework OpenGL -framework GLUT -Wno-deprecated-declarations
./Oceano
```

## Controles

| Entrada | Acción |
|---|---|
| Arrastrar con botón izquierdo | Orientar la cámara con `yaw` y `pitch` |
| Flechas arriba/abajo | Avanzar o retroceder sobre el plano XZ |
| Flechas izquierda/derecha | Desplazamiento lateral (*strafe*) |
| `+` / `-` | Acercar o alejar mediante el campo de visión |
| `W` / `S` | Aumentar o reducir la velocidad de las olas |
| Espacio | Pausar o reanudar la animación |
| `T` | Activar o desactivar la textura del océano |
| `L` | Activar o desactivar la iluminación de la geometría |
| `Q` / `Esc` | Salir |

La altura de la cámara permanece fija y su posición se limita a una zona segura
dentro del disco oceánico para evitar revelar la parte inferior de la malla.

## Estructura principal

```text
include/
├── Boat.h
├── Environment.h
├── Island.h
├── Lighthouse.h
├── Ocean.h
├── Wave.h
└── WPoint.h
src/
├── Boat.cpp
├── Environment.cpp
├── Island.cpp
├── Lighthouse.cpp
├── Ocean.cpp
├── Wave.cpp
└── main.cpp
data/
├── spectrum.txt
└── spectrum_realista.txt
assets/textures/
└── ocean.tga
```

## Modelo de olas

La altura de un punto `(x, z)` en el instante `t` se obtiene con:

```text
h(x,z,t) = Σ Ai · cos(ki · (x·cos(di) + z·sin(di)) - 2π·fi·t + pi)
```

`Ocean::update(t)` aplica esta función a la malla y recalcula sus normales.
`Ocean::getHeightAt(x,z,t)` evalúa la misma función para que otros objetos, como
el bote, permanezcan sincronizados con la superficie.
