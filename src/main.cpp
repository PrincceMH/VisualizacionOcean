#include <GL/glut.h>
#include "../include/Ocean.h"

// Creamos una instancia global de nuestro océano
// 50 filas, 50 columnas, con una separación de 0.5 entre cada punto

Ocean miOceano(50, 50, 0.5f);


void display() {
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    
    gluLookAt(0.0, 10.0, 20.0,  
              0.0, 0.0, 0.0,   
              0.0, 1.0, 0.0);

    
    glColor3f(0.0f, 0.8f, 1.0f);

    miOceano.draw();

    glutSwapBuffers();
}


void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = w * 1.0f / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
   
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    
    glutCreateWindow("Proyecto Oceano");

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    
    return 0;
}