#include "mpi_types.h"
#include <stddef.h>

MPI_Datatype createParticleMPIType() {
    MPI_Datatype particleType;
    MPI_Datatype types[8] = { MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_INT, MPI_FLOAT, MPI_INT };
    int blocklen[8] = { 3, 3, 4, 1, 1, 1, 3 * 3, 1 };
    MPI_Aint offsets[8];

    offsets[0] = offsetof(Particle, position);
    offsets[1] = offsetof(Particle, velocity);
    offsets[2] = offsetof(Particle, color);
    offsets[3] = offsetof(Particle, life);
    offsets[4] = offsetof(Particle, size);
    offsets[5] = offsetof(Particle, active);
    offsets[6] = offsetof(Particle, trail);
    offsets[7] = offsetof(Particle, trailIndex);

    // Create the MPI datatype
    MPI_Type_create_struct(8, blocklen, offsets, types, &particleType);
    MPI_Type_commit(&particleType);
    return particleType;
}