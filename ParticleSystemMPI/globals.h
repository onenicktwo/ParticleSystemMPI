#ifndef GLOBALS_H
#define GLOBALS_H

#include "particle.h"

#define MAX_PARTICLES 1000000

extern Particle particles[MAX_PARTICLES];
extern int window_width;
extern int window_height;
extern float mouseX;   // X coordinate of the mouse in OpenGL space
extern float mouseY;   // Y coordinate of the mouse in OpenGL space
extern int attractParticles;  // 1 for attraction, -1 for repulsion, 0 for no effect

#endif