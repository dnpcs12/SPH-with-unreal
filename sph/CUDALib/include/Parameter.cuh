#ifndef PARTICLES_KERNEL_H
#define PARTICLES_KERNEL_H

typedef unsigned int uint;

struct SimParams
{

    float3 gravity;
    float particleRadius;

    uint3 gridSize;
    uint numCells;
    float3 worldOrigin;
    float3 cellSize;

    float boundaryDamping;

    float temp;
    float Cs;
    float3 boundary;
    float timeStep;
    float restDensity;
    float mass;
    float boundaryMass;
    float viscosity;
    float rigidyViscosity;
    float surfaceTension;
    float threshold;
    float gasStiffness;
    float H;
    float H2;
    float Wpoly6;
    float Wspiky_Grad;
    float Wviscosity_Lapl;
};
#endif