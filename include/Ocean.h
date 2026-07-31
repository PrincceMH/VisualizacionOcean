#ifndef OCEAN_H
#define OCEAN_H

#include <vector>
#include <string>
#include "WPoint.h"
#include "Wave.h"

class Ocean {
private:
    int rows;             // numero de anillos: i=0 centro ... i=rows-1 borde exterior
    int cols;             // numero de sectores + 1 (el ultimo duplica al primero: costura)
    float maxRadius;      // radio del disco de agua

    std::vector<std::vector<WPoint>> mesh;
    
    // Vector dinamico para almacenar las olas cargadas del txt
    std::vector<Wave> waves;
    
    // ID para guardar la textura cargada en OpenGL
    unsigned int textureID;

    // Bandera para activar/desactivar el dibujado de la textura
    bool showTexture;

    // Altura de cresta mas alta del frame actual (referencia para la espuma)
    float maxHeight;

    // Multiplicador de amplitud de las olas (1 = normal, >1 = mar embravecido)
    float waveScale;

    // true durante la 2da pasada (dibuja solo la espuma, sumada sobre el agua)
    bool foamPass;

    float foamFactor(float y) const;    // cuanta espuma segun la altura del vertice

    void drawTriangles();
    void emitVertex(const WPoint& p);   // envia un vertice con color de espuma

public:
    // Constructor: numRings anillos, numSectors sectores, radius = radio del disco
    Ocean(int numRings, int numSectors, float radius);

    // inicializacion
    void initMesh();
    bool loadWaves(const std::string& filename);
    bool loadTexture(const std::string& filename);

    // Activa o desactiva la textura del agua (control por teclado)
    void setTexture(bool on) { showTexture = on; }

    // Escala la fuerza de las olas (amplitud). 1 = normal, ~2 = tormenta.
    void setWaveScale(float s) { waveScale = s; }

    void update(float time);
    void computeNormals();
    void draw();
    // Evalua h(x,z,t) directamente con la formula de olas, sin depender
    // de que (x,z) coincida con un vertice de la malla. Util para que
    // otros objetos (barco, boyas, etc.) sepan a que altura flotar.
    float getHeightAt(float x, float z, float time) const;
};

#endif
