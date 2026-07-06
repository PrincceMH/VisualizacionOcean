#include "../include/Ocean.h"
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <cmath>

static const float PI = 3.14159265358979323846f;

//dimensiones y malla
Ocean::Ocean(int r, int c, float s) : rows(r), cols(c), spacing(s), textureID(0) {
    initMesh();
}

// cuadricula de puntos plana
void Ocean::initMesh() {
    mesh.resize(rows, std::vector<WPoint>(cols));

    // punto de inicio para que el oceano quede centrado en la pantalla
    float startX = -((cols - 1) * spacing) / 2.0f;
    float startZ = -((rows - 1) * spacing) / 2.0f;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            mesh[i][j].x = startX + j * spacing;
            mesh[i][j].y = 0.0f;
            mesh[i][j].z = startZ + i * spacing;

            // Normal
            mesh[i][j].nx = 0.0f;
            mesh[i][j].ny = 1.0f;
            mesh[i][j].nz = 0.0f;

            // Coordenadas UV para la textura
            mesh[i][j].s = (float)j / (cols - 1);
            mesh[i][j].t = (float)i / (rows - 1);
        }
    }
}

// triangulos de la malla
void Ocean::drawTriangles() {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            const WPoint& p00 = mesh[i][j];
            const WPoint& p10 = mesh[i + 1][j];
            const WPoint& p01 = mesh[i][j + 1];
            const WPoint& p11 = mesh[i + 1][j + 1];

            glNormal3f(p00.nx, p00.ny, p00.nz);
            glTexCoord2f(p00.s, p00.t);
            glVertex3f(p00.x, p00.y, p00.z);

            glNormal3f(p10.nx, p10.ny, p10.nz);
            glTexCoord2f(p10.s, p10.t);
            glVertex3f(p10.x, p10.y, p10.z);

            glNormal3f(p01.nx, p01.ny, p01.nz);
            glTexCoord2f(p01.s, p01.t);
            glVertex3f(p01.x, p01.y, p01.z);




            glNormal3f(p01.nx, p01.ny, p01.nz);
            glTexCoord2f(p01.s, p01.t);
            glVertex3f(p01.x, p01.y, p01.z);

            glNormal3f(p10.nx, p10.ny, p10.nz);
            glTexCoord2f(p10.s, p10.t);
            glVertex3f(p10.x, p10.y, p10.z);

            glNormal3f(p11.nx, p11.ny, p11.nz);
            glTexCoord2f(p11.s, p11.t);
            glVertex3f(p11.x, p11.y, p11.z);
        }
    }
    glEnd();
}


void Ocean::draw() {

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawTriangles();

    // Wireframe encima glPolygonOffset
    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.25f); // negro al 25% de opacidad

        drawTriangles();
    glPopAttrib();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Lee archivo
bool Ocean::loadWaves(const std::string& filename) {

    std::vector<std::string> posiblesRutas = {
        filename,
        "./" + filename,
        "../" + filename,
        "../../" + filename,
        "./src/" + filename,
        "./data/" + filename
    };

    std::ifstream file;
    std::string rutaUsada;
    for (const std::string& ruta : posiblesRutas) {
        file.open(ruta);
        if (file.is_open()) {
            rutaUsada = ruta;
            break;
        }
    }

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo de olas: " << filename
                   << " (revisa que este en la misma carpeta desde donde "
                   << "se ejecuta el programa)" << std::endl;
        std::cerr << "Se usaran olas por defecto." << std::endl;

        waves.clear();
        waves.emplace_back(0.5f, 0.15f, 30.0f  * PI / 180.0f, 0.0f);
        waves.emplace_back(0.3f, 0.25f, 90.0f  * PI / 180.0f, 60.0f  * PI / 180.0f);
        waves.emplace_back(0.2f, 0.40f, 150.0f * PI / 180.0f, 120.0f * PI / 180.0f);
        return false;
    }

    waves.clear();
    float amp, freq, dirDeg, phaseDeg;
    while (file >> amp >> freq >> dirDeg >> phaseDeg) {
        // Direccion y fase se guardan en radianes, asi las usa la formula
        float dirRad = dirDeg * PI / 180.0f;
        float phaseRad = phaseDeg * PI / 180.0f;
        waves.emplace_back(amp, freq, dirRad, phaseRad);
    }
    file.close();

    std::cout << "Se cargaron " << waves.size() << " olas desde " << rutaUsada << std::endl;
    return !waves.empty();
}

bool Ocean::loadTexture(const std::string& filename) {

    std::cout << "loadTexture: pendiente de implementar (" << filename << ")" << std::endl;
    return true;
}

// Aplica h(x, z, t) = suma_i Ai * cos(ki*(x*cos(di) + z*sin(di)) - 2*pi*fi*t + pi)
// a cada punto de la malla, y recalcula las normales
void Ocean::update(float time) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            float x = mesh[i][j].x;
            float z = mesh[i][j].z;
            float height = 0.0f;

            for (const Wave& w : waves) {
                float A = w.getAmplitude();
                float f = w.getFrequency();
                float d = w.getDirection();
                float p = w.getPhase();
                float k = w.getWaveNumber();

                height += A * cosf(k * (x * cosf(d) + z * sinf(d)) - 2.0f * PI * f * time + p);
            }

            mesh[i][j].y = height;
        }
    }

    computeNormals();
}

void Ocean::computeNormals() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            float hL = mesh[i][j > 0 ? j - 1 : j].y;
            float hR = mesh[i][j < cols - 1 ? j + 1 : j].y;
            float hD = mesh[i > 0 ? i - 1 : i][j].y;
            float hU = mesh[i < rows - 1 ? i + 1 : i][j].y;

            float nx = -(hR - hL);
            float nz = -(hU - hD);
            float ny = 2.0f * spacing;

            float len = sqrtf(nx * nx + ny * ny + nz * nz);
            if (len > 0.00001f) {
                mesh[i][j].nx = nx / len;
                mesh[i][j].ny = ny / len;
                mesh[i][j].nz = nz / len;
            }
        }
    }
}
