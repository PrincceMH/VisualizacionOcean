#ifndef WPOINT_H
#define WPOINT_H

struct WPoint {
    // Coordenadas espaciales
    float x, y, z;
    
    // Coordenadas de la normal 
    float nx, ny, nz;
    
    // Coordenadas de textura 
    float s, t;

    // Constructor por defecto
    WPoint() : x(0.0f), y(0.0f), z(0.0f), 
               nx(0.0f), ny(1.0f), nz(0.0f), 
               s(0.0f), t(0.0f) {}
};

#endif