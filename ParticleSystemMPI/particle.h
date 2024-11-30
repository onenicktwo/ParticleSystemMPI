#ifndef PARTICLE_H
#define PARTICLE_H

typedef struct {
    float position[3];
    float velocity[3];
    float color[4];
    float life;
    float size;
    int active;
    float trail[3][3];
    int trailIndex;   
} Particle;

void initParticles(void);
void updateParticles(void);
void addVortexForce(float centerX, float centerY, float strength, float radius);

#endif