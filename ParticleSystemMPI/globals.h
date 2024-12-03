#ifndef GLOBALS_H
#define GLOBALS_H

#include "particle.h"

#define MAX_PARTICLES 10000

extern Particle particles[MAX_PARTICLES];
extern int window_width;
extern int window_height;
extern float mouseX;
extern float mouseY;
extern int attractParticles;

#endif