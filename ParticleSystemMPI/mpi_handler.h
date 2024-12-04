#ifndef MPI_HANDLER_H
#define MPI_HANDLER_H

#include <mpi.h>
#include "particle.h"

void initMPIHandler();
void synchronizeParticles();

#endif