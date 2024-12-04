#include "mpi_handler.h"
#include "globals.h"
#include "mpi.h"

void initMPI(int argc, char** argv, int* rank, int* size) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, rank);
    MPI_Comm_size(MPI_COMM_WORLD, size);
}

void distributeParticles(int rank, int size) {
    int particles_per_process = MAX_PARTICLES / size;
    int remainder = MAX_PARTICLES % size;

    // Calculate the start and end indices for the current rank
    int start_index = rank * particles_per_process + (rank < remainder ? rank : remainder);
    int end_index = start_index + particles_per_process + (rank < remainder ? 1 : 0);

    // Initialize particles for this rank
    for (int i = start_index; i < end_index; i++) {
        initParticle(&particles[i], i % EMITTER_COUNT); // Assuming initParticle initializes a particle
    }
}

void exchangeBoundaryParticles(int rank, int size) {
    int particles_per_process = MAX_PARTICLES / size;
    int start_index = rank * particles_per_process;
    int end_index = (rank + 1) * particles_per_process;

    // Exchange boundary particles with neighboring processes
    if (rank > 0) {
        MPI_Send(&particles[start_index], sizeof(Particle), MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&particles[start_index - 1], sizeof(Particle), MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (rank < size - 1) {
        MPI_Send(&particles[end_index - 1], sizeof(Particle), MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&particles[end_index], sizeof(Particle), MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void gatherParticles(int rank, int size) {
    MPI_Gather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
        particles, MAX_PARTICLES / size * sizeof(Particle), MPI_BYTE,
        0, MPI_COMM_WORLD);
}