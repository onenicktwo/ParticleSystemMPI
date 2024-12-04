#ifndef MPI_TYPE_H
#define MPI_TYPE_H

#include <mpi.h>
#include "particle.h"

extern MPI_Datatype Particle_MPI_Type;

// Function declarations
void createParticleMPIType(void);
void cleanupParticleMPIType(void);

#endif