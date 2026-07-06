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
    std::vector<Wave> waves;
    unsigned int textureID;
    void drawTriangles();

public:
    Ocean(int r, int c, float s);

    void initMesh();
    bool loadWaves(const std::string& filename);
    bool loadTexture(const std::string& filename);

    void update(float time);
    void computeNormals();
    void draw();
};

#endif
