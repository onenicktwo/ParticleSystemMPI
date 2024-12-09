#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <math.h>
#include <stddef.h>

#define NUM_PARTICLES 1000
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define FPS 60
#define NUM_FRAMES 100000

typedef struct {
    float x, y;
    float vx, vy;
    float ax, ay;
} Particle;

Particle* particles = NULL;
Particle* local_particles = NULL;
int num_particles_per_proc;
int rank, size;

MPI_Datatype particle_type;

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
            particles[i].ax = 0.0f;
            particles[i].ay = 0.0f;
        }
    }
}

void updateParticles() {
    for (int i = 0; i < num_particles_per_proc; i++) {
        local_particles[i].x += local_particles[i].vx;
        local_particles[i].y += local_particles[i].vy;
        local_particles[i].vx += local_particles[i].ax;
        local_particles[i].vy += local_particles[i].ay;
        local_particles[i].ax = 0.0f;
        local_particles[i].ay = -0.1f;

        if (local_particles[i].x < 0 || local_particles[i].x > WINDOW_WIDTH) local_particles[i].vx *= -1;
        if (local_particles[i].y < 0 || local_particles[i].y > WINDOW_HEIGHT) local_particles[i].vy *= -1;
    }

    // Exchange boundary particles with neighboring processes
    MPI_Request request[4];
    MPI_Status status[4];

    // Send and receive boundary particles
    if (rank > 0) {
        MPI_Isend(&local_particles[0], 1, particle_type, rank - 1, 0, MPI_COMM_WORLD, &request[0]);
        MPI_Irecv(&local_particles[0], 1, particle_type, rank - 1, 0, MPI_COMM_WORLD, &request[1]);
    }
    if (rank < size - 1) {
        MPI_Isend(&local_particles[num_particles_per_proc - 1], 1, particle_type, rank + 1, 0, MPI_COMM_WORLD, &request[2]);
        MPI_Irecv(&local_particles[num_particles_per_proc - 1], 1, particle_type, rank + 1, 0, MPI_COMM_WORLD, &request[3]);
    }

    // Wait for boundary communication to complete
    if (rank > 0) {
        MPI_Wait(&request[0], &status[0]);
        MPI_Wait(&request[1], &status[1]);
    }
    if (rank < size - 1) {
        MPI_Wait(&request[2], &status[2]);
        MPI_Wait(&request[3], &status[3]);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Define the derived datatype for Particle
    MPI_Type_create_struct(
        6,
        (int[]) {
        1, 1, 1, 1, 1, 1
    },
        (MPI_Aint[]) {
        offsetof(Particle, x),
            offsetof(Particle, y),
            offsetof(Particle, vx),
            offsetof(Particle, vy),
            offsetof(Particle, ax),
            offsetof(Particle, ay)
    },
        (MPI_Datatype[]) {
        MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT
    },
        & particle_type
    );
    MPI_Type_commit(&particle_type);

    // Calculate particles per process
    int* recv_counts = NULL;
    int* displs = NULL;

    int particles_per_process = NUM_PARTICLES / size;
    int remainder = NUM_PARTICLES % size;

    if (rank == 0) {
        recv_counts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            recv_counts[i] = particles_per_process + (i < remainder ? 1 : 0);
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recv_counts[i - 1];
        }
    }

    num_particles_per_proc = particles_per_process + (rank < remainder ? 1 : 0);
    local_particles = (Particle*)malloc(num_particles_per_proc * sizeof(Particle));
    if (local_particles == NULL) {
        fprintf(stderr, "Rank %d: Failed to allocate memory for local_particles\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) {
        initParticles();
    }

    // Scatter particles to processes
    MPI_Scatterv(
        particles,
        recv_counts,
        displs,
        particle_type,
        local_particles,
        num_particles_per_proc,
        particle_type,
        0,
        MPI_COMM_WORLD
    );

    // Start the timer
    double start_time = MPI_Wtime();

    // Main simulation loop
    for (int frame = 0; frame < NUM_FRAMES; frame++) {
        updateParticles();
    }

    // Stop the timer
    double end_time = MPI_Wtime();
    double elapsed_time = end_time - start_time;

    // Gather all particles to the root process
    MPI_Gatherv(
        local_particles,
        num_particles_per_proc,
        particle_type,
        particles,
        recv_counts,
        displs,
        particle_type,
        0,
        MPI_COMM_WORLD
    );

    // Calculate performance metrics
    double max_time;
    MPI_Reduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double updates_per_second = (NUM_PARTICLES * NUM_FRAMES) / max_time;
        printf("Simulation completed in %.2f seconds\n", max_time);
        printf("Updates per second: %.2f\n", updates_per_second);
        printf("Particles updated: %d\n", NUM_PARTICLES * NUM_FRAMES);
    }

    // Clean up
    free(local_particles);
    if (rank == 0) {
        free(particles);
        free(recv_counts);
        free(displs);
    }

    MPI_Type_free(&particle_type);
    MPI_Finalize();
    return 0;
}