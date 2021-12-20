
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "Parameter.cuh"
#include "defines.h"

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
        uint   boundaryStartIndex, //수정중
        uint   numCells,
        float  timeStep);

    void sortParticles(uint* dGridParticleHash, uint* dGridParticleIndex, uint numParticles);

    //-------마칭큐브-------------------///
    void allocateTextures(uint** d_edgeTable, uint** d_triTable, uint** d_numVertsTable);
    void destroyAllTextureObjects();
    void ThrustScanWrapper(unsigned int* output, unsigned int* input, unsigned int numElements);



    void launch_classifyVoxel(dim3 grid,
        dim3 threads,
        uint* voxelVerts,
        uint* voxelOccupied,
        uint3 gridSize,
        uint3 gridSizeShift,
        uint3 gridSizeMask,
        uint numVoxels,
        float3 voxelSize,
        float isoValue,
        float* SortedPositions,
        uint* CellStarts,
        uint* CellEnds,
        uint* gridParticleIndex);

    void launch_compactVoxels(dim3 grid, dim3 threads, uint* compactedVoxelArray, uint* voxelOccupied, uint* voxelOccupiedScan, uint numVoxels);

    void launch_generateTriangles(dim3 grid, dim3 threads,
        float4* pos, float4* norm, uint* compactedVoxelArray, uint* numVertsScanned,
        uint3 gridSize, uint3 gridSizeShift, uint3 gridSizeMask,
        float3 voxelSize, float isoValue, uint activeVoxels, uint maxVerts,
        float4* SortedPositions,
        uint* CellStarts,
        uint* CellEnds,
        uint* gridParticleIndex);

}

