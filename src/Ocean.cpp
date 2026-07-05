#include "../include/Ocean.h"
#include <GL/glut.h> 
#include <iostream>

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


bool Ocean::loadWaves(const std::string& filename) { return true; }
bool Ocean::loadTexture(const std::string& filename) { return true; }
void Ocean::update(float time) {}
void Ocean::computeNormals() {}