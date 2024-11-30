#include "particle.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "globals.h"

#define GRAVITY -9.81f
#define INITIAL_VELOCITY 5.0f
#define TIME_STEP 0.016f
#define MOUSE_ATTRACTION_STRENGTH 0.5f  // Adjust the strength of attraction/repulsion
#define MAX_ATTRACTION_DISTANCE 2.0f    // Maximum distance for mouse attraction effect

extern Particle particles[MAX_PARTICLES];

// Store mouse position
float mouseX = 0.0f;
float mouseY = 0.0f;
int attractParticles = 1;  // 1 to attract, -1 to repel

void initParticle(Particle* p) {
    p->position[0] = (float)(rand() % 100 - 50) / 50.0f;
    p->position[1] = -1.0f;
    p->position[2] = 0.0f;

    p->velocity[0] = (float)(rand() % 100 - 50) / 50.0f;
    p->velocity[1] = INITIAL_VELOCITY + ((float)(rand() % 100) / 100.0f);
    p->velocity[2] = 0.0f;

    p->color[0] = 0.0f;  // Initial color (blue)
    p->color[1] = 0.0f;
    p->color[2] = 1.0f;
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

        // Calculate distance from particle to mouse position for interaction
        float dx = mouseX - particles[i].position[0];
        float dy = mouseY - particles[i].position[1];
        float distance = sqrtf(dx * dx + dy * dy);

        // Apply mouse attraction or repulsion if within a certain distance
        if (distance < MAX_ATTRACTION_DISTANCE && distance > 0.1f) {
            float attractionForce = MOUSE_ATTRACTION_STRENGTH / distance * attractParticles;
            particles[i].velocity[0] += attractionForce * dx;
            particles[i].velocity[1] += attractionForce * dy;
        }

        // Update particle position
        particles[i].position[0] += particles[i].velocity[0] * TIME_STEP;
        particles[i].position[1] += particles[i].velocity[1] * TIME_STEP;
        particles[i].position[2] += particles[i].velocity[2] * TIME_STEP;

        // Apply gravity
        particles[i].velocity[1] += GRAVITY * TIME_STEP;

        // Update color based on velocity magnitude (color gradient effect)
        float velocityMagnitude = sqrtf(particles[i].velocity[0] * particles[i].velocity[0] +
            particles[i].velocity[1] * particles[i].velocity[1] +
            particles[i].velocity[2] * particles[i].velocity[2]);

        // Map velocity to color gradient: from blue (slow) to red (fast)
        float velocityFactor = velocityMagnitude / (INITIAL_VELOCITY * 2.0f); // Normalize velocity factor
        particles[i].color[0] = velocityFactor;    // Red channel increases with velocity
        particles[i].color[1] = 0.0f;              // Green channel stays 0 for simplicity
        particles[i].color[2] = 1.0f - velocityFactor; // Blue channel decreases with velocity

        // Lifespan fading effect
        particles[i].color[3] = particles[i].life; // Alpha value decreases as particle life decreases
        particles[i].life -= TIME_STEP;

        // If particle's life runs out or falls below the ground, reinitialize it
        if (particles[i].life <= 0.0f || particles[i].position[1] < -1.0f) {
            initParticle(&particles[i]);
        }
    }
}