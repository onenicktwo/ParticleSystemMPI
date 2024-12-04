#ifndef MPI_HANDLER_H
#define MPI_HANDLER_H

#include "mpi.h"

void initMPI(int* rank, int* size);
void finalizeMPI(void);
void distributeParticles(int rank, int size);
void exchangeBoundaryParticles(int rank, int size);
void gatherParticles(int rank, int size);

#endif