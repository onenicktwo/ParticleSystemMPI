#ifndef GLOBALS_H
#define GLOBALS_H

#include "particle.h"
#include <mpi.h>

#define MAX_PARTICLES 1000
#define EMITTER_COUNT 5
#define VORTEX_STRENGTH 0.5f

extern Particle particles[MAX_PARTICLES];
extern int window_width;
extern int window_height;
extern float mouseX;
extern float mouseY;
extern int attractParticles;
extern int rank;
extern int size;
extern int emitter_count;
extern float currentVortexStrength;

// MPI-related globals
extern MPI_Datatype MPI_PARTICLE;

#endif