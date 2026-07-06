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

// Dibuja la malla conectando los puntos con triangulos
void Ocean::draw() {
    // Por ahora, dibujamos solo las líneas para ver bien la cuadrícula
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            // Triangulo 1
            glVertex3f(mesh[i][j].x, mesh[i][j].y, mesh[i][j].z);
            glVertex3f(mesh[i+1][j].x, mesh[i+1][j].y, mesh[i+1][j].z);
            glVertex3f(mesh[i][j+1].x, mesh[i][j+1].y, mesh[i][j+1].z);

            // Triangulo 2
            glVertex3f(mesh[i][j+1].x, mesh[i][j+1].y, mesh[i][j+1].z);
            glVertex3f(mesh[i+1][j].x, mesh[i+1][j].y, mesh[i+1][j].z);
            glVertex3f(mesh[i+1][j+1].x, mesh[i+1][j+1].y, mesh[i+1][j+1].z);
        }
    }
    glEnd();
    
    // Restaurar a modo relleno
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


// Lee amplitud, direccion y frecuencia por linea desde el archivo de espectro;
// la fase se genera aleatoriamente como indica el enunciado del laboratorio.
bool Ocean::loadWaves(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir " << filename << std::endl;
        return false;
    }

    waves.clear();
    float amplitude, direction, frequency;
    while (file >> amplitude >> direction >> frequency) {
        float phase = static_cast<float>(rand()) / RAND_MAX * 2.0f * PI;
        waves.push_back(Wave(amplitude, frequency, direction, phase));
    }

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

// Normal por diferencias centrales de la altura respecto a los vecinos en x y z.
void Ocean::computeNormals() {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            float hL = (j > 0) ? mesh[i][j - 1].y : mesh[i][j].y;
            float hR = (j < cols - 1) ? mesh[i][j + 1].y : mesh[i][j].y;
            float hD = (i > 0) ? mesh[i - 1][j].y : mesh[i][j].y;
            float hU = (i < rows - 1) ? mesh[i + 1][j].y : mesh[i][j].y;

            float dx = (hR - hL) / (2.0f * spacing);
            float dz = (hU - hD) / (2.0f * spacing);

            float nx = -dx;
            float ny = 1.0f;
            float nz = -dz;
            float len = sqrtf(nx * nx + ny * ny + nz * nz);

            mesh[i][j].nx = nx / len;
            mesh[i][j].ny = ny / len;
            mesh[i][j].nz = nz / len;
        }
    }
}