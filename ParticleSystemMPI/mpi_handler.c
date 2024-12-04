#include "mpi_handler.h"
#include "globals.h"

static MPI_Datatype MPI_PARTICLE;
static int syncCounter = 0;
#define SYNC_INTERVAL 10

void initMPIHandler() {
    // Create MPI datatype for Particle
    int blocklengths[] = { 3, 3, 4, 1, 1, 1, 9, 1 };
    MPI_Datatype types[] = { MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_INT, MPI_FLOAT, MPI_INT };
    MPI_Aint offsets[8];

    offsets[0] = offsetof(Particle, position);
    offsets[1] = offsetof(Particle, velocity);
    offsets[2] = offsetof(Particle, color);
    offsets[3] = offsetof(Particle, life);
    offsets[4] = offsetof(Particle, size);
    offsets[5] = offsetof(Particle, active);
    offsets[6] = offsetof(Particle, trail);
    offsets[7] = offsetof(Particle, trailIndex);

    MPI_Type_create_struct(8, blocklengths, offsets, types, &MPI_PARTICLE);
    MPI_Type_commit(&MPI_PARTICLE);
}

void synchronizeParticles() {
    syncCounter++;
    if (syncCounter < SYNC_INTERVAL) return;
    syncCounter = 0;

    int particlesPerProcess = MAX_PARTICLES / size;
    int startIndex = rank * particlesPerProcess;
    int endIndex = (rank == size - 1) ? MAX_PARTICLES : (rank + 1) * particlesPerProcess;
    int particlesToSend = endIndex - startIndex;

    // Gather all particles to rank 0
    if (rank == 0) {
        // Receive buffer for rank 0
        MPI_Gather(MPI_IN_PLACE, particlesToSend, MPI_PARTICLE,
            particles, particlesToSend, MPI_PARTICLE,
            0, MPI_COMM_WORLD);
    }
    else {
        // Send buffer for other ranks
        MPI_Gather(&particles[startIndex], particlesToSend, MPI_PARTICLE,
            NULL, 0, MPI_PARTICLE,
            0, MPI_COMM_WORLD);
    }

    // Broadcast the updated particles to all processes
    MPI_Bcast(particles, MAX_PARTICLES, MPI_PARTICLE, 0, MPI_COMM_WORLD);
}

void broadcastInteractionData() {
    float interactionData[4] = { mouseX, mouseY, (float)attractParticles, currentVortexStrength };
    MPI_Bcast(interactionData, 4, MPI_FLOAT, 0, MPI_COMM_WORLD);

    mouseX = interactionData[0];
    mouseY = interactionData[1];
    attractParticles = (int)interactionData[2];
    currentVortexStrength = interactionData[3];
}