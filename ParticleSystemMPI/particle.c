#include "particle.h"
#include <stdlib.h>
#include <time.h>
#include "globals.h"

#define GRAVITY -9.81f
#define INITIAL_VELOCITY 5.0f
#define TIME_STEP 0.016f

extern Particle particles[MAX_PARTICLES];

void initParticle(Particle* p) {
    p->position[0] = (float)(rand() % 100 - 50) / 50.0f;
    p->position[1] = -1.0f;
    p->position[2] = 0.0f;

    p->velocity[0] = (float)(rand() % 100 - 50) / 50.0f;
    p->velocity[1] = INITIAL_VELOCITY + ((float)(rand() % 100) / 100.0f);
    p->velocity[2] = 0.0f;

    p->color[0] = (float)(rand() % 100) / 100.0f;
    p->color[1] = (float)(rand() % 100) / 100.0f;
    p->color[2] = (float)(rand() % 100) / 100.0f;
    p->color[3] = 1.0f;

    p->life = 1.0f + ((float)(rand() % 100) / 100.0f);
    p->size = 2.0f + ((float)(rand() % 30) / 10.0f);
    p->active = 1;
}

void initParticles() {
    srand(time(NULL));
    for (int i = 0; i < MAX_PARTICLES; i++) {
        initParticle(&particles[i]);
    }
}

void updateParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        particles[i].position[0] += particles[i].velocity[0] * TIME_STEP;
        particles[i].position[1] += particles[i].velocity[1] * TIME_STEP;
        particles[i].position[2] += particles[i].velocity[2] * TIME_STEP;

        particles[i].velocity[1] += GRAVITY * TIME_STEP;
        particles[i].color[3] = particles[i].life;
        particles[i].life -= TIME_STEP;

        if (particles[i].life <= 0.0f || particles[i].position[1] < -1.0f) {
            initParticle(&particles[i]);
        }
    }
}