
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "Parameter.cuh"


extern "C"
{
    void allocateArray(void** devPtr, size_t size);
    void freeArray(void* devPtr);

    void threadSync();

    void copyArrayFromDevice(void* host, const void* device, int size);
    void copyArrayToDevice(void* device, const void* host, int offset, int size);
    void setParameters(SimParams* hostParams);

    
    void computeBoundaryVolume(float* boundaryVolumes, float* Pos, uint boundaryStart, uint curStartIndex, uint count);

    void integrateSystem(float* pos,
        float* vel,
        uint rigidyCount,
        float3* rigidyMovePos,
        float3* rigidyVel,
        uint numFluidParticles,
        uint boundaryStartIndex,
        uint* numBoundaryParticles);

    void calcHash(uint* gridParticleHash,
        uint* gridParticleIndex,
        float* pos,
        int    numParticles);

    void reorderDataAndFindCellStart(uint* cellStart,
        uint* cellEnd,
        float* sortedPos,
        float* sortedVel,
        uint* gridParticleHash,
        uint* gridParticleIndex,
        float* oldPos,
        float* oldVel,
        uint   numParticles,
        uint   numCells);

    void collide(float* newVel,
        float* newMassDensity,
        float* newPressure,
        float* boundaryVolume,
        float* sortedPos,
        float* sortedVel,
        uint* gridParticleIndex,
        uint* cellStart,
        uint* cellEnd,
        uint   numParticles,
        uint   numFluidParticles,
        uint   boundaryStartIndex, //¼öÁ¤Áß
        uint   numCells,
        float  timeStep);

    void sortParticles(uint* dGridParticleHash, uint* dGridParticleIndex, uint numParticles);

}
