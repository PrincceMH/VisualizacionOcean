#ifndef BOAT_H
#define BOAT_H

class Ocean;

// Representa un barco simple que flota sobre la superficie del oceano.
// No tiene su propia malla animada: en cada frame le pregunta a Ocean
// la altura del agua bajo el casco (proa y popa) para posicionarse e
// inclinarse de forma coherente con las olas.
class Boat {
private:
    float x, z;        // posicion horizontal (fija por ahora)
    float length;       // largo del casco (eje Z local)
    float width;        // ancho del casco (eje X local)
    float hullHeight;    // alto del casco

public:
    Boat(float posX, float posZ, float len = 2.2f, float wid = 0.9f, float hgt = 0.5f);

    // Dibuja el barco flotando en la posicion (x,z), consultando a
    // "ocean" la altura del agua en el instante "time"
    void draw(const Ocean& ocean, float time) const;
};

#endif
