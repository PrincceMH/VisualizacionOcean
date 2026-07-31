#ifndef ISLAND_H
#define ISLAND_H

#include <vector>

// Representa una isla/costa estatica (no se anima con el tiempo): una
// grilla de "anillos" (centro -> borde) y "divisiones angulares" que sube
// desde el nivel del mar (y=0) hasta una altura maxima en el centro. El
// contorno NO es un circulo perfecto: usa dos radios (X y Z) para poder
// alargarla en una direccion, mas una perturbacion angular determinista
// que le da un contorno irregular, tipo costa natural.
class Island {
private:
    struct IPoint {
        float x, y, z;
        float nx, ny, nz;
    };

    std::vector<std::vector<IPoint>> mesh; // [anillo][division angular]

    int rings;
    int slices;
    float radiusX;      // "radio" a lo largo del eje X (ancho de la costa)
    float radiusZ;       // "radio" a lo largo del eje Z (cuanto se mete al mar)
    float maxHeight;
    float centerX;
    float centerZ;
    float irregularity;  // 0 = elipse perfecta, mas alto = contorno mas irregular
    int noiseSeed;

    void buildMesh();
    void computeNormals();

    // Factor multiplicador del radio segun el angulo (1.0 = elipse base),
    // para romper la forma circular/eliptica perfecta
    float shapeFactor(float ang) const;

    // Color segun la altura relativa (0 = borde/orilla, 1 = cima)
    void colorForHeight(float t) const;

    // Dibuja una roca irregular (bajo poligono) en una posicion dada,
    // usada para sembrar rocas alrededor de la base de la isla
    void drawRock(float cx, float cy, float cz, float scale, int seed) const;

public:
    // radX / radZ: extension maxima en X y en Z (no tienen que ser iguales;
    // usa radX grande y radZ chico para una costa larga y angosta).
    // irregularityAmount: 0.0 = silueta suave (elipse), ~0.3-0.4 = costa
    // bien irregular con entrantes y salientes.
    Island(float cx, float cz, float radX, float radZ, float maxH = 2.2f,
           int ringsCount = 16, int slicesCount = 48,
           float irregularityAmount = 0.25f, int seed = 7);

    void draw() const;

    // Utiles para ubicar otros objetos sobre la isla (ej. un faro en la cima)
    float getCenterX() const { return centerX; }
    float getCenterZ() const { return centerZ; }
    float getPeakHeight() const { return maxHeight; }
};

#endif
