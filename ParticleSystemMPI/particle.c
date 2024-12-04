#include "particle.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "globals.h"
#include "mpi_handler.h"

#define GRAVITY -9.81f
#define INITIAL_VELOCITY 5.0f
#define TIME_STEP 0.016f
#define MOUSE_ATTRACTION_STRENGTH 0.5f
#define MAX_ATTRACTION_DISTANCE 2.0f
#define VORTEX_RADIUS 0.5f
#define M_PI 3.14159265358979323846

typedef struct {
    float position[2];
    float direction[2];
    float spread;
} Emitter;

Emitter emitters[EMITTER_COUNT];

void initEmitters() {
    emitters[0] = (Emitter){ {-0.8f, -1.0f}, {0.5f, 1.0f}, 30.0f };
    emitters[1] = (Emitter){ {0.0f, -1.0f}, {0.0f, 1.0f}, 15.0f };
    emitters[2] = (Emitter){ {0.8f, -1.0f}, {-0.5f, 1.0f}, 30.0f };
    emitters[3] = (Emitter){ {-0.8f, -0.5f}, {0.5f, 0.5f}, 45.0f };
    emitters[4] = (Emitter){ {0.8f, -0.5f}, {-0.5f, 0.5f}, 45.0f };
}

void initParticle(Particle* p, int emitterIndex) {
    Emitter* e = &emitters[emitterIndex];
    float angle = (rand() % (int)(e->spread * 2) - e->spread) * M_PI / 180.0f;
    float speed = INITIAL_VELOCITY + ((float)(rand() % 100) / 100.0f);

    p->position[0] = e->position[0];
    p->position[1] = e->position[1];
    p->position[2] = 0.0f;

    p->velocity[0] = (e->direction[0] * cosf(angle) - e->direction[1] * sinf(angle)) * speed;
    p->velocity[1] = (e->direction[0] * sinf(angle) + e->direction[1] * cosf(angle)) * speed;
    p->velocity[2] = 0.0f;

    p->color[0] = 0.0f;
    p->color[1] = 0.0f;
    p->color[2] = 1.0f;
    p->color[3] = 1.0f;

    p->life = 1.0f + ((float)(rand() % 100) / 100.0f);
    p->size = 2.0f + ((float)(rand() % 30) / 10.0f);
    p->active = 1;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            p->trail[i][j] = p->position[j];
        }
    }
    p->trailIndex = 0;
}

void initParticles() {
    srand(time(NULL) + rank); // Use different seeds for each process
    initEmitters();

    int particlesPerProcess = MAX_PARTICLES / size;
    int startIndex = rank * particlesPerProcess;
    int endIndex = (rank == size - 1) ? MAX_PARTICLES : (rank + 1) * particlesPerProcess;

    for (int i = startIndex; i < endIndex; i++) {
        initParticle(&particles[i], i % EMITTER_COUNT);
    }

    // Synchronize initial particle states across all processes
    synchronizeParticles();
}

void addVortexForce(float centerX, float centerY, float strength, float radius) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        float dx = particles[i].position[0] - centerX;
        float dy = particles[i].position[1] - centerY;
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < radius) {
            float factor = 1.0f - (distance / radius);
            float vx = -dy * factor * strength;
            float vy = dx * factor * strength;
            particles[i].velocity[0] += vx * TIME_STEP;
            particles[i].velocity[1] += vy * TIME_STEP;
        }
    }
}

void updateParticles() {
    broadcastInteractionData(); // Ensure all processes have up-to-date interaction data

    int particlesPerProcess = MAX_PARTICLES / size;
    int startIndex = rank * particlesPerProcess;
    int endIndex = (rank == size - 1) ? MAX_PARTICLES : (rank + 1) * particlesPerProcess;

    for (int i = startIndex; i < endIndex; i++) {
        if (!particles[i].active) {
            initParticle(&particles[i], i % EMITTER_COUNT);
            continue;
        }

        // Update trail
        particles[i].trailIndex = (particles[i].trailIndex + 1) % 3;
        for (int j = 0; j < 3; j++) {
            particles[i].trail[particles[i].trailIndex][j] = particles[i].position[j];
        }

        // Mouse attraction/repulsion
        float dx = mouseX - particles[i].position[0];
        float dy = mouseY - particles[i].position[1];
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < MAX_ATTRACTION_DISTANCE && distance > 0.1f) {
            float attractionForce = MOUSE_ATTRACTION_STRENGTH / distance * attractParticles;
            particles[i].velocity[0] += attractionForce * dx;
            particles[i].velocity[1] += attractionForce * dy;
        }

        // Update position and apply gravity
        particles[i].position[0] += particles[i].velocity[0] * TIME_STEP;
        particles[i].position[1] += particles[i].velocity[1] * TIME_STEP;
        particles[i].position[2] += particles[i].velocity[2] * TIME_STEP;

        particles[i].velocity[1] += GRAVITY * TIME_STEP;

        // Color gradient
        float velocityMagnitude = sqrtf(particles[i].velocity[0] * particles[i].velocity[0] +
            particles[i].velocity[1] * particles[i].velocity[1] +
            particles[i].velocity[2] * particles[i].velocity[2]);

        float velocityFactor = velocityMagnitude / (INITIAL_VELOCITY * 2.0f);
        particles[i].color[0] = velocityFactor;
        particles[i].color[1] = 0.0f;
        particles[i].color[2] = 1.0f - velocityFactor;

        // Lifespan and fading
        particles[i].color[3] = particles[i].life;
        particles[i].life -= TIME_STEP;

        // Reinitialize particle if it's dead or out of bounds
        if (particles[i].life <= 0.0f || particles[i].position[1] < -1.0f ||
            particles[i].position[0] < -1.0f || particles[i].position[0] > 1.0f) {
            initParticle(&particles[i], i % EMITTER_COUNT);
        }
    }

    addVortexForce(0.0f, 0.0f, currentVortexStrength, VORTEX_RADIUS);

    // Synchronize particles across all processes (less frequently)
    synchronizeParticles();
}