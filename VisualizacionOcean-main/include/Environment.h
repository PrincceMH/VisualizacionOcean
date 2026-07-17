#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

class Environment {
private:
    float sunX;
    float sunY;
    float sunSize;
    float sunDirX;
    float sunDirY;
    float sunDirZ;

    void drawSky(int w, int h) const;
    void drawSun(int w, int h) const;

public:
    Environment();
    void initLight() const;
    void applyLight() const;
    void draw(int w, int h) const;
};

#endif
