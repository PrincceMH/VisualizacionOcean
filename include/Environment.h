#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class Environment {
private:
    float toSunX;
    float toSunY;
    float toSunZ;
    float skyRadius;
    float sunCoreRadius;
    float sunGlowRadius;

    // 0 = dia despejado (sol), 1 = tormenta (cielo oscuro, luna, luz tenue)
    float stormFactor;

    void drawSky() const;
    void drawSun() const;

public:
    Environment();
    void initLight() const;
    void applyLight() const;
    void draw(float cameraX, float cameraY, float cameraZ) const;

    // Ajusta el clima: interpola el cielo, el sol->luna y la iluminacion.
    void setStorm(float t) { stormFactor = (t < 0.0f) ? 0.0f : (t > 1.0f ? 1.0f : t); }
};

#endif
