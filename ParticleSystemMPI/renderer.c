#include "renderer.h"
#include "particle.h"
#include "GL/glut.h"
#include "stdlib.h"
#include "globals.h"
#include <mpi.h>

#define M_PI 3.14159265358979323846
#define VORTEX_STRENGTH 0.5f

extern float currentVortexStrength;

void display() {
    if (rank == 0) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_POINT_SMOOTH);

        // Draw particle trails using lines
        glBegin(GL_LINES);
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) continue;

            glColor4f(particles[i].color[0], particles[i].color[1], particles[i].color[2], particles[i].color[3] * 0.3f);
            for (int j = 0; j < 2; j++) {
                int idx1 = (particles[i].trailIndex + j) % 3;
                int idx2 = (particles[i].trailIndex + j + 1) % 3;
                glVertex3fv(particles[i].trail[idx1]);
                glVertex3fv(particles[i].trail[idx2]);
            }
        }
        glEnd();

        // Draw particles using points
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
    // Update particles on all processes
    updateParticles();

    // Exchange boundary particles
    exchangeBoundaryParticles(rank, size);

    // Gather particles to root process
    gatherParticles(rank, size);

    // Synchronize all processes
    MPI_Barrier(MPI_COMM_WORLD);

    // Redraw the scene (only on root process)
    if (rank == 0) {
        glutPostRedisplay();
        glutTimerFunc(16, timer, 0);
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (rank == 0) {
        switch (key) {
        case 27:
            MPI_Abort(MPI_COMM_WORLD, 0);  // Terminate all MPI processes
            exit(0);
            break;
        case 'r': // Reset particles
            initParticles();
            break;
        case 'v': // Toggle vortex on/off
            currentVortexStrength = (currentVortexStrength == 0.0f) ? VORTEX_STRENGTH : 0.0f;
            break;
        default:
            attractParticles = 0;
            break;
        }
        // Broadcast the changes to other processes
        MPI_Bcast(&currentVortexStrength, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&attractParticles, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
}

void mouseMotion(int x, int y) {
    if (rank == 0) {
        float aspectRatio = (float)window_width / (float)window_height;
        mouseX = ((float)x / (float)window_width) * 2.0f - 1.0f;
        mouseY = 1.0f - ((float)y / (float)window_height) * 2.0f;

        // Adjust for aspect ratio
        if (window_width > window_height) {
            mouseX *= aspectRatio;
        }
        else {
            mouseY /= aspectRatio;
        }

        // Broadcast mouse position to other processes
        float mousePos[2] = { mouseX, mouseY };
        MPI_Bcast(mousePos, 2, MPI_FLOAT, 0, MPI_COMM_WORLD);
    }
    else {
        // Non-root processes receive mouse position
        float mousePos[2];
        MPI_Bcast(mousePos, 2, MPI_FLOAT, 0, MPI_COMM_WORLD);
        mouseX = mousePos[0];
        mouseY = mousePos[1];
    }
}

void mouse(int button, int state, int x, int y) {
    if (rank == 0) {
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

        // Broadcast attractParticles to other processes
        MPI_Bcast(&attractParticles, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else {
        // Non-root processes receive attractParticles
        MPI_Bcast(&attractParticles, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
}