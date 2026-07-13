## Compilación y ejecución

### macOS

```bash
g++ -std=c++11 src/main.cpp src/Ocean.cpp src/Wave.cpp -o Oceano -framework OpenGL -framework GLUT -Wno-deprecated-declarations
./Oceano
```

### Windows (MinGW + freeglut)

```bash
g++ src/main.cpp src/Ocean.cpp src/Wave.cpp -o Oceano.exe -lopengl32 -lglu32 -lfreeglut
Oceano.exe
```
# Simulación de la Superficie del Océano

Simulación en tiempo real de la superficie del océano mediante una malla regular de triángulos, animada con la superposición de ondas sinusoidales (modelo *Sum of Sines*), con iluminación y material especular para dar apariencia de agua.

Desarrollado en C++ con OpenGL / GLUT.

---

## Estado del proyecto

| Requisito del enunciado | Estado |
|---|---|
| Malla regular de triángulos sobre el plano XZ | ✅ Implementado |
| Altura animada con suma de olas senoidales h(x,z,t) | ✅ Implementado |
| Clases `Wave`, `WPoint`, `Ocean` | ✅ Implementado |
| Carga de olas desde archivo de espectro | ✅ Implementado |
| Cálculo de normales por vértice (promedio de caras) | ✅ Implementado |
| Iluminación (ambiental + difusa + especular) | ✅ Implementado |
| Material con componente especular (brillos) | ⏳ Pendiente |
| Mapeo de textura (`ocean.tga`) | ⏳ Pendiente |

---

## Fundamento matemático

La altura de cada punto de la malla en la posición `(x, z)` y en el instante `t` es la suma de `N` olas individuales:

```
h(x, z, t) = Σ  Ai · cos( ki·(x·cos(di) + z·sin(di)) − 2π·fi·t + pi )
             i=1..N
```

Donde `Ai` es la amplitud, `fi` la frecuencia, `di` la dirección (radianes) y `pi` la fase de cada ola. El número de onda `ki` se calcula con la relación de dispersión de aguas profundas:

```
ki = 4π² · fi² / 9.81
```

---

## Estructura de clases

- **`Wave`** — Representa una ola individual: amplitud, frecuencia, dirección, fase, y el cálculo de su número de onda `ki`.
- **`WPoint`** — Representa un vértice de la malla: posición `(x, y, z)`, normal `(nx, ny, nz)` y coordenadas de textura `(s, t)`.
- **`Ocean`** — Construye la malla, mantiene la lista de olas, actualiza la altura y las normales en cada frame, y dibuja la superficie.

### Métodos principales de `Ocean`

| Método | Función |
|---|---|
| `initMesh()` | Genera la cuadrícula plana inicial (altura 0), centrada en el origen, con sus coordenadas de textura. |
| `loadWaves(archivo)` | Lee un espectro de olas desde un archivo de texto (`amplitud`, `dirección`, `frecuencia` por línea) y genera la fase de cada ola aleatoriamente. Si el archivo no se encuentra, prueba varias rutas relativas y, si aun así falla, usa un set de olas por defecto para que la superficie no quede plana. |
| `update(t)` | Recorre cada punto de la malla, aplica `h(x,z,t)` sumando todas las olas del espectro, y recalcula las normales. |
| `computeNormals()` | Calcula la normal de cada vértice **promediando las normales de las caras (triángulos) vecinas**: por cada triángulo se obtiene su normal con producto cruz de dos aristas, se acumula en sus 3 vértices, y al final se normaliza la suma en cada vértice. |
| `draw()` | Dibuja la superficie rellena (afectada por la luz) y, encima, la malla de triángulos en modo wireframe semitransparente (sin iluminación), para distinguir la triangulación sin perder el color del agua. |

---

## Archivo de espectro de olas (`data/spectrum.txt`)

Cada línea define una ola con el formato:

```
amplitud   dirección(rad)   frecuencia
0.504      -1.571           0.225
0.484      -1.190           0.202
...
```

La fase de cada ola se genera aleatoriamente al cargar el espectro, tal como indica el enunciado. Se pueden agregar, quitar o modificar líneas de este archivo para cambiar el aspecto del oleaje sin tocar el código.

---


> **Importante:** el programa debe ejecutarse desde la carpeta raíz del proyecto (la que contiene `data/spectrum.txt`), ya que `main.cpp` carga el espectro con la ruta relativa `"data/spectrum.txt"`.

---

## Estructura de carpetas

```
OCEANO/
├── include/
│   ├── Ocean.h
│   ├── Wave.h
│   └── WPoint.h
├── src/
│   ├── main.cpp
│   ├── Ocean.cpp
│   └── Wave.cpp
└── data/
    └── spectrum.txt
```
