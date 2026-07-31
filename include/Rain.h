#ifndef RAIN_H
#define RAIN_H

#include <vector>

// Lluvia de tormenta con billboards (quads que encaran a la camara), basada en
// la tecnica del lab de Billboards: los ejes derecha/arriba se leen de la matriz
// de vista para que cada gota mire siempre al observador. El volumen de lluvia
// sigue a la camara para que siempre llueva donde se mira.
class Rain {
private:
    struct Drop { float x, y, z, vy; };
    std::vector<Drop> drops;
    unsigned int texID;   // textura procedural de la gota
    float area;           // medio ancho de la zona de lluvia (alrededor de la camara)
    float topOffset;      // altura sobre la camara desde donde caen las gotas

    void respawn(Drop& d, float camX, float camY, float camZ) const;

public:
    Rain(int count = 1200);
    void initTexture();                                   // crea la textura de la gota
    void update(float dt, float camX, float camY, float camZ);
    void draw(float intensity) const;                     // intensity 0..1 (fade de tormenta)
};

#endif
