#include <GL/glut.h>
#include <mpi.h>
#include "particle.h"
#include "renderer.h"
#include "globals.h"
#include "mpi_handler.h"

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Initialize particles on all processes
    initParticles();

    // Distribute particles among processes
    distributeParticles(rank, size);

    if (rank == 0) {
        // Initialize GLUT on root process
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

        // Set clear color to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Start the simulation loop (timer-based)
        glutTimerFunc(16, timer, 0);

        // Start main loop on root process
        glutMainLoop();
    }
    else {
        // Non-root processes enter a simulation loop
        while (1) {
            updateParticles();
            exchangeBoundaryParticles(rank, size);
            gatherParticles(rank, size);
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}