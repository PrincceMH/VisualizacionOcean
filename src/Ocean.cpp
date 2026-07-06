#include "../include/Ocean.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>

static const float PI = 3.14159265358979323846f;

//guarda las dimensiones y llama a generar la malla
Ocean::Ocean(int r, int c, float s) : rows(r), cols(c), spacing(s), textureID(0) {
    initMesh();
}

// Genera la cuadricula de puntos plana
void Ocean::initMesh() {
    mesh.resize(rows, std::vector<WPoint>(cols));
    
    // Calculamos el punto de inicio para que el oceano quede centrado en la pantalla
    float startX = -((cols - 1) * spacing) / 2.0f;
    float startZ = -((rows - 1) * spacing) / 2.0f;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            mesh[i][j].x = startX + j * spacing;
            mesh[i][j].y = 0.0f; 
            mesh[i][j].z = startZ + i * spacing;
            
            // Normal apuntando hacia arriba (eje Y) por defecto
            mesh[i][j].nx = 0.0f; 
            mesh[i][j].ny = 1.0f; 
            mesh[i][j].nz = 0.0f;
            
            // Coordenadas UV para la textura (valores de 0.0 a 1.0)
            mesh[i][j].s = (float)j / (cols - 1);
            mesh[i][j].t = (float)i / (rows - 1);
        }
    }
}

// Emite los triangulos de la malla (posicion, normal, texcoord por vertice)
void Ocean::drawTriangles() {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            const WPoint& p00 = mesh[i][j];
            const WPoint& p10 = mesh[i + 1][j];
            const WPoint& p01 = mesh[i][j + 1];
            const WPoint& p11 = mesh[i + 1][j + 1];

            // Triangulo 1: p00 - p10 - p01
            glNormal3f(p00.nx, p00.ny, p00.nz);
            glTexCoord2f(p00.s, p00.t);
            glVertex3f(p00.x, p00.y, p00.z);

            glNormal3f(p10.nx, p10.ny, p10.nz);
            glTexCoord2f(p10.s, p10.t);
            glVertex3f(p10.x, p10.y, p10.z);

            glNormal3f(p01.nx, p01.ny, p01.nz);
            glTexCoord2f(p01.s, p01.t);
            glVertex3f(p01.x, p01.y, p01.z);

            // Triangulo 2: p01 - p10 - p11
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

// Dibuja la superficie rellena (afectada por la iluminacion/material) y,
// encima, la malla de triangulos en modo wireframe translucido, sin luz,
// para poder ver a la vez el brillo del agua y la estructura de la malla.
void Ocean::draw() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawTriangles();

    glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_POLYGON_BIT | GL_COLOR_BUFFER_BIT | GL_LIGHTING_BIT);
        glDisable(GL_LIGHTING); // las lineas de la malla se ven mejor sin sombreado
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


// Lee amplitud, direccion y frecuencia por linea desde el archivo de espectro;
// la fase se genera aleatoriamente como indica el enunciado del laboratorio.
bool Ocean::loadWaves(const std::string& filename) {
    // El directorio de trabajo cambia segun desde donde se ejecute el
    // programa (terminal, IDE, carpeta de build, etc). Probamos varias
    // rutas comunes para no depender de eso.
    std::vector<std::string> posiblesRutas = {
        filename,
        "./" + filename,
        "../" + filename,
        "../../" + filename,
        "./data/spectrum.txt",
        "../data/spectrum.txt",
        "data/spectrum.txt"
    };

    std::ifstream file;
    std::string rutaUsada;
    for (const std::string& ruta : posiblesRutas) {
        file.open(ruta.c_str());
        if (file.is_open()) {
            rutaUsada = ruta;
            break;
        }
        file.clear();
    }

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo de espectro: " << filename
                   << " (revisa que este en la misma carpeta desde donde "
                   << "se ejecuta el programa)" << std::endl;
        std::cerr << "Se usaran olas por defecto para que la simulacion no quede plana." << std::endl;

        waves.clear();
        waves.push_back(Wave(0.5f, 0.15f, 30.0f  * PI / 180.0f, 0.0f));
        waves.push_back(Wave(0.3f, 0.25f, 90.0f  * PI / 180.0f, 60.0f  * PI / 180.0f));
        waves.push_back(Wave(0.2f, 0.40f, 150.0f * PI / 180.0f, 120.0f * PI / 180.0f));
        return false;
    }

    waves.clear();
    float amplitude, direction, frequency;
    while (file >> amplitude >> direction >> frequency) {
        float phase = static_cast<float>(rand()) / RAND_MAX * 2.0f * PI;
        waves.push_back(Wave(amplitude, frequency, direction, phase));
    }
    file.close();

    std::cout << "Se cargaron " << waves.size() << " olas desde " << rutaUsada << std::endl;
    return !waves.empty();
}

bool Ocean::loadTexture(const std::string& filename) { return true; }

// h(x,z,t) = suma de Ai * cos(ki*(x*cos(di) + z*sin(di)) - 2*pi*fi*t + pi)
void Ocean::update(float time) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            float x = mesh[i][j].x;
            float z = mesh[i][j].z;
            float height = 0.0f;

            for (size_t w = 0; w < waves.size(); ++w) {
                const Wave& wave = waves[w];
                float k = wave.getWaveNumber();
                float theta = k * (x * cosf(wave.getDirection()) + z * sinf(wave.getDirection()))
                              - 2.0f * PI * wave.getFrequency() * time
                              + wave.getPhase();
                height += wave.getAmplitude() * cosf(theta);
            }

            mesh[i][j].y = height;
        }
    }

    computeNormals();
}

// Calcula la normal de cada vertice promediando las normales de las caras
// (triangulos) que lo tocan, tal como pide el enunciado:
// 1) Para cada triangulo se calcula su normal de cara con el producto
//    cruz de dos de sus aristas.
// 2) Esa normal se suma en cada uno de los 3 vertices del triangulo
//    (sin normalizar antes, para que los triangulos mas grandes pesen
//    un poco mas, lo cual da un resultado mas suave).
// 3) Al final, se normaliza la suma acumulada en cada vertice.
void Ocean::computeNormals() {
    // 1. Reiniciar el acumulador de cada vertice
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            mesh[i][j].nx = 0.0f;
            mesh[i][j].ny = 0.0f;
            mesh[i][j].nz = 0.0f;
        }
    }

    // 2. Recorrer cada celda de la malla (2 triangulos por celda) y
    //    acumular la normal de cara en sus 3 vertices
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            WPoint& p00 = mesh[i][j];
            WPoint& p10 = mesh[i + 1][j];
            WPoint& p01 = mesh[i][j + 1];
            WPoint& p11 = mesh[i + 1][j + 1];

            // --- Triangulo 1: p00 - p10 - p01 ---
            float e1x = p10.x - p00.x, e1y = p10.y - p00.y, e1z = p10.z - p00.z;
            float e2x = p01.x - p00.x, e2y = p01.y - p00.y, e2z = p01.z - p00.z;

            float fnx = e1y * e2z - e1z * e2y;
            float fny = e1z * e2x - e1x * e2z;
            float fnz = e1x * e2y - e1y * e2x;

            p00.nx += fnx; p00.ny += fny; p00.nz += fnz;
            p10.nx += fnx; p10.ny += fny; p10.nz += fnz;
            p01.nx += fnx; p01.ny += fny; p01.nz += fnz;

            // --- Triangulo 2: p01 - p10 - p11 ---
            e1x = p10.x - p01.x; e1y = p10.y - p01.y; e1z = p10.z - p01.z;
            e2x = p11.x - p01.x; e2y = p11.y - p01.y; e2z = p11.z - p01.z;

            fnx = e1y * e2z - e1z * e2y;
            fny = e1z * e2x - e1x * e2z;
            fnz = e1x * e2y - e1y * e2x;

            p01.nx += fnx; p01.ny += fny; p01.nz += fnz;
            p10.nx += fnx; p10.ny += fny; p10.nz += fnz;
            p11.nx += fnx; p11.ny += fny; p11.nz += fnz;
        }
    }

    // 3. Normalizar el promedio acumulado en cada vertice
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            WPoint& p = mesh[i][j];
            float len = sqrtf(p.nx * p.nx + p.ny * p.ny + p.nz * p.nz);
            if (len > 0.00001f) {
                p.nx /= len;
                p.ny /= len;
                p.nz /= len;
            } else {
                p.nx = 0.0f; p.ny = 1.0f; p.nz = 0.0f;
            }
        }
    }
}