#ifndef OCEAN_H
#define OCEAN_H

#include <vector>
#include <string>
#include "WPoint.h"
#include "Wave.h"

class Ocean {
private:
    int rows;             // Filas de la malla (eje Z)
    int cols;             // Columnas de la malla (eje X)
    float spacing;        // Distancia entre cada punto
    
    // Matriz 2D que contiene todos los vértices
    std::vector<std::vector<WPoint>> mesh;
    
    // Vector dinámico para almacenar las olas cargadas del txt
    std::vector<Wave> waves;
    
    // ID para guardar la textura cargada en OpenGL
    unsigned int textureID;
    void drawTriangles();

public:
    // Constructor
    Ocean(int r, int c, float s);

    // Métodos de inicialización
    void initMesh();
    bool loadWaves(const std::string& filename);
    bool loadTexture(const std::string& filename);

    // Métodos de ciclo de vida (animación y gráficos)
    void update(float time);
    void computeNormals();
    void draw();
};

#endif