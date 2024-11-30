#include "renderer.h"
#include "particle.h"
#include <GL/glut.h>
#include <stdlib.h>
#include "globals.h"

extern Particle particles[MAX_PARTICLES];
extern int window_width;
extern int window_height;
extern float mouseX;  // External variable for mouse X position
extern float mouseY;  // External variable for mouse Y position
extern int attractParticles;  // External variable for attract/repel toggle

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);

    glBegin(GL_POINTS);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        glPointSize(particles[i].size);
        glColor4fv(particles[i].color);
        glVertex3fv(particles[i].position);
    }
    glEnd();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    window_width = w;
    window_height = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)w / (float)h;
    if (w <= h)
        glOrtho(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect, -1.0, 1.0);
    else
        glOrtho(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    updateParticles();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);  // 16 ms for ~60 FPS
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // ESC key
        exit(0);
        break;
    case 'r': // Reset particles
        initParticles();
        break;
    case 'a': // Toggle to attract particles
        attractParticles = 1;
        break;
    case 'd': // Toggle to repel particles
        attractParticles = -1;
        break;
    default:
        attractParticles = 0;
        break;
    }
}

// Mouse motion callback to track mouse position
void mouseMotion(int x, int y) {
    // Convert mouse coordinates to OpenGL world coordinates
    float aspectRatio = (float)window_width / (float)window_height;
    mouseX = ((float)x / (float)window_width) * 2.0f - 1.0f;  // Normalize x to [-1, 1]
    mouseY = 1.0f - ((float)y / (float)window_height) * 2.0f; // Normalize y to [-1, 1]

    // Adjust for aspect ratio
    if (window_width > window_height) {
        mouseX *= aspectRatio;
    }
    else {
        mouseY /= aspectRatio;
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Attraction mode on mouse click
        attractParticles = 1;
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        // Stop attraction on release
        attractParticles = 0;
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        // Repel particles with right button
        attractParticles = -1;
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
        // Stop repelling on release
        attractParticles = 0;
    }
}