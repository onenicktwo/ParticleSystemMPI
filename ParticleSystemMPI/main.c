#include <GL/glut.h>
#include <mpi.h>
#include <Windows.h>
#include "particle.h"
#include "renderer.h"
#include "globals.h"
#include "mpi_handler.h"

int running = 1;

void cleanup() {
    running = 0;
}

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Initialize MPI handler
    initMPIHandler();

    // Initialize particles (all processes)
    initParticles();

    if (rank == 0) {
        // Initialize GLUT
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutInitWindowSize(window_width, window_height);
        glutCreateWindow("Particle System");

        // Set up display functions
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(keyboard);
        glutMouseFunc(mouse);
        glutMotionFunc(mouseMotion);
        glutTimerFunc(0, timer, 0);

        // Set clear color to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Set up cleanup function
        atexit(cleanup);

        // Start main loop (only for rank 0)
        glutMainLoop();
    }
    else {
        // Other processes enter a computation loop
        while (running) {
            updateParticles();
            Sleep(16); // Sleep for approximately 16ms (60 FPS)
        }
    }

    // Finalize MPI
    MPI_Type_free(&MPI_PARTICLE);
    MPI_Finalize();
    return 0;
}