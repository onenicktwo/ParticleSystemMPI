#ifndef PARTICLE_H
#define PARTICLE_H

typedef struct {
    float position[3];
    float velocity[3];
    float color[4];
    float life;
    float size;
    int active;
} Particle;

void initParticle(Particle* p);
void initParticles(void);
void updateParticles(void);

#endif