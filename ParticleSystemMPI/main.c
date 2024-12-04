#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <GL/glut.h>
#include <time.h>
#include <math.h>

#define NUM_PARTICLES 500000
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 60

typedef struct {
    float x, y;
    float vx, vy;
} Particle;

Particle* particles = NULL;
Particle* local_particles = NULL;
int num_particles_per_proc;
int rank, size;
double lastTime;
int frameCount = 0;
double fps = 0.0;

void initParticles() {
    if (rank == 0) {
        particles = (Particle*)malloc(NUM_PARTICLES * sizeof(Particle));
        if (particles == NULL) {
            fprintf(stderr, "Failed to allocate memory for particles\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        for (int i = 0; i < NUM_PARTICLES; i++) {
            particles[i].x = rand() % WINDOW_WIDTH;
            particles[i].y = rand() % WINDOW_HEIGHT;
            particles[i].vx = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
            particles[i].vy = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
        }
    }
}

void updateParticles() {
    for (int i = 0; i < num_particles_per_proc; i++) {
        local_particles[i].x += local_particles[i].vx;
        local_particles[i].y += local_particles[i].vy;

        if (local_particles[i].x < 0 || local_particles[i].x > WINDOW_WIDTH) local_particles[i].vx *= -1;
        if (local_particles[i].y < 0 || local_particles[i].y > WINDOW_HEIGHT) local_particles[i].vy *= -1;
    }

    // Gather all particles to the root process
    MPI_Gather(local_particles, num_particles_per_proc * sizeof(Particle), MPI_BYTE,
        particles, num_particles_per_proc * sizeof(Particle), MPI_BYTE,
        0, MPI_COMM_WORLD);
}

// Display function for OpenGL
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        glVertex2f(particles[i].x / WINDOW_WIDTH * 2 - 1, particles[i].y / WINDOW_HEIGHT * 2 - 1);
    }
    glEnd();

    frameCount++;
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    if (currentTime - lastTime > 1.0) {
        fps = frameCount / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;
    }

    char fpsString[30];
    sprintf_s(fpsString, sizeof(fpsString), "FPS: %.2f", fps);

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-0.98f, 0.85f);
    glVertex2f(-0.98f, 0.95f);
    glVertex2f(-0.7f, 0.95f);
    glVertex2f(-0.7f, 0.85f);
    glEnd();
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.95f, 0.9f); 
    for (char* c = fpsString; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glutSwapBuffers();
}

// Timer function to control the update rate
void timer(int value) {
    updateParticles();
    if (rank == 0) {
        glutPostRedisplay();
    }
    glutTimerFunc(1000 / FPS, timer, 0);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    num_particles_per_proc = (NUM_PARTICLES + size - 1) / size;
    local_particles = (Particle*)malloc(num_particles_per_proc * sizeof(Particle));
    if (local_particles == NULL) {
        fprintf(stderr, "Failed to allocate memory for local_particles\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) {
        initParticles();
    }

    MPI_Scatter(particles, num_particles_per_proc * sizeof(Particle), MPI_BYTE,
        local_particles, num_particles_per_proc * sizeof(Particle), MPI_BYTE,
        0, MPI_COMM_WORLD);

    if (rank == 0) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        glutCreateWindow("Particle System");

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glPointSize(2.0f);

        glutDisplayFunc(display);
        glutTimerFunc(1000 / FPS, timer, 0);
        lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

        glutMainLoop();
    }
    else {
        while (1) {
            updateParticles();
        }
    }

    free(local_particles);
    if (rank == 0) {
        free(particles);
    }

    MPI_Finalize();
    return 0;
}