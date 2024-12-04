#include "mpi_type.h"

MPI_Datatype Particle_MPI_Type;

void createParticleMPIType(void) {
    int blocklengths[5] = { 3, 3, 4, 1, 1 }; // Number of elements in each block
    MPI_Aint displacements[5];
    MPI_Datatype types[5] = { MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_INT };

    // Calculate displacements using offsetof
    displacements[0] = offsetof(Particle, position);
    displacements[1] = offsetof(Particle, velocity);
    displacements[2] = offsetof(Particle, color);
    displacements[3] = offsetof(Particle, life);
    displacements[4] = offsetof(Particle, active);

    MPI_Type_create_struct(5, blocklengths, displacements, types, &Particle_MPI_Type);
    MPI_Type_commit(&Particle_MPI_Type);
}

void cleanupParticleMPIType(void) {
    MPI_Type_free(&Particle_MPI_Type);
}