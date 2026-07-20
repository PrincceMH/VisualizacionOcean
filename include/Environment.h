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

    void drawSky() const;
    void drawSun() const;

public:
    Environment();
    void initLight() const;
    void applyLight() const;
    void draw(float cameraX, float cameraY, float cameraZ) const;
};

#endif
