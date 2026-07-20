#ifndef OCEAN_H
#define OCEAN_H

#include <vector>
#include <string>
#include "WPoint.h"
#include "Wave.h"

class Ocean {
private:
    int rows;             
    int cols;             
    float spacing;        
    
    std::vector<std::vector<WPoint>> mesh;
    
    // Vector dinamico para almacenar las olas cargadas del txt
    std::vector<Wave> waves;
    
    // ID para guardar la textura cargada en OpenGL
    unsigned int textureID;
    
    void drawTriangles();

public:
    // Constructor
    Ocean(int r, int c, float s);

    // inicializacion
    void initMesh();
    bool loadWaves(const std::string& filename);
    bool loadTexture(const std::string& filename);

    void update(float time);
    void computeNormals();
    void draw();

    // Evalua h(x,z,t) directamente con la formula de olas, sin depender
    // de que (x,z) coincida con un vertice de la malla. Util para que
    // otros objetos (barco, boyas, etc.) sepan a que altura flotar.
    float getHeightAt(float x, float z, float time) const;
};

#endif