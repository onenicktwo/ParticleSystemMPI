#include "globals.h"

Particle particles[MAX_PARTICLES];
int window_width = 800;
int window_height = 600;
float mouseX = 0.0f;
float mouseY = 0.0f;
int attractParticles = 0;
int rank;
int size;
int emitter_count = EMITTER_COUNT;
int currentVortexStrength = VORTEX_STRENGTH;

// MPI-related globals
MPI_Datatype MPI_PARTICLE;