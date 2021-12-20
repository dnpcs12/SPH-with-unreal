// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <vector>
#include "CoreMinimal.h"
#include "CUDA_SPH.cuh"
#include "Parameter.cuh"
#include "GameFramework/Actor.h"
#include "MyStaticMeshActor.h"
#include "ProceduralMeshComponent.h"


#include "SPHsystemWihCuda.generated.h"


UCLASS()
class SPH_API ASPHsystemWihCuda : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPHsystemWihCuda();
	
	UFUNCTION(BlueprintCallable)
	void AddRigidySphere(float r, FVector initpos);

	UFUNCTION(BlueprintCallable)
	void AddRigidyHemisphere(float r, FVector initpos,bool up = true);

	UFUNCTION(BlueprintCallable)
	void AddRigidyCylinder(float r, int height, FVector initpos);

	UFUNCTION(BlueprintCallable)
	void SpawnFluidParticles(FVector initpos);

	UFUNCTION(BlueprintCallable)
	void ResistRigidy(AMyStaticMeshActor* rigidyActor, int type = 1);

	UFUNCTION(BlueprintCallable)
	void ResetFluidPosition();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual ~ASPHsystemWihCuda();

	void initSystem();
	void finalize();

	void step();

	void computeIsoface();
	void createIsosurface();

	void updateRigidyPosition();
	void updatePosition();

	void tempProcess();
	void addrigidy(FVector initpos);

	float3 FVectorToFloat3(FVector vec);

	void addParticleToCuda(uint index, FVector pos, FVector vel = {0,0,0});

public:	
	const float myPI = 3.141592f;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere)
		UInstancedStaticMeshComponent* particles = nullptr;
	UPROPERTY(EditAnywhere)
		UInstancedStaticMeshComponent* bparticles = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UProceduralMeshComponent* ParticleProceduralMeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		class UMaterialInterface* machingCubeMaterial = nullptr;

	UPROPERTY(EditAnywhere)
	    TArray<AMyStaticMeshActor*> rigidyActors;
	//UPROPERTY(EditAnywhere)
	//    UStaticMeshComponent* stoneMesh;
	//UPROPERTY(EditAnywhere)
	//	UStaticMeshComponent* stoneMesh2;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
		int32 sizeX;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
		int32 sizeY;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
		int32 sizeZ;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0))
		int maxBoundaryParticle;

	UPROPERTY(EditAnywhere)
		FVector boundary;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float particleRadius;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float elasticity;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float timeStep;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float restDensity;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float mass;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float viscosity;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float rigidyViscosity;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float surfaceTension;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float threshold;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
		float gasStiffness;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.01))
		float supportRadius;

	UPROPERTY(EditAnywhere,meta = (ClampMin = 0.04))
	    float boundaryR;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.01))
		float boudaryMass;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1))
		float Cs;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.01))
	    float temp;
	UPROPERTY(EditAnywhere)
	    float isoValue;

	UPROPERTY(EditAnywhere)
	    bool renderMarchingCube = true;

	UPROPERTY(EditAnywhere)
		bool spawnMode = true;

	UPROPERTY(EditAnywhere)
	    int spawnX = 3;

	UPROPERTY(EditAnywhere)
		int spawnY = 3;

	UPROPERTY(EditAnywhere)
	    float scaleCorrectedValue;

protected:
	float timeC;

	bool spawnEnd = false;
	bool test = false;
	uint bnum = 0;

	FVector mParticleScale;
	uint m_maxParticles = 0; 
	uint m_totalParticles = 0; // fluid particles + boundary particles


	uint m_maxFluidParticles = 0;
	uint m_numFluidsParticles = 0;

	uint m_numBoundaryParticles =0;
	
	uint m_rigidyCount = 0;
	std::vector<uint> m_numBoundarys;

	std::vector<float3> m_rigidyMovingPos;
	std::vector<float3> m_rigidyVel;
	std::vector<FVector> m_rigidyPos;


	// CPU data
	float* m_hPos;              // particle positions
	float* m_hVel;              // particle velocities

	uint* m_hParticleHash;
	uint* m_hCellStart;
	uint* m_hCellEnd;

	// GPU data
	float* m_dPos;
	float* m_dVel;
	float* m_dMassDensity; // 내가 추가
	float* m_dPressure; // 내가 추가
	float* m_dBoundaryVolume; // for boundary handling.

	float* m_dSortedPos;
	float* m_dSortedVel;

	// grid data for sorting method
	uint* m_dGridParticleHash; // grid hash value for each particle
	uint* m_dGridParticleIndex;// particle index for each particle
	uint* m_dCellStart;        // index of start of each cell in sorted list
	uint* m_dCellEnd;          // index of end of cell

	// params
	SimParams m_params;
	uint3 m_gridSize;
	uint m_numGridCells;


	//마칭큐브 
	uint3 gridSizeLog2 = make_uint3(6u, 6u, 6u);
	uint3 gridSizeShift = make_uint3(0, gridSizeLog2.x, gridSizeLog2.x + gridSizeLog2.y);
	uint3 gridSize = make_uint3(1 << gridSizeLog2.x, 1 << gridSizeLog2.y, 1 << gridSizeLog2.z);
	uint3 gridSizeMask = make_uint3(gridSize.x - 1, gridSize.y - 1, gridSize.z - 1);

	float3 mc_voxelSize = make_float3(2.0f / gridSize.x, 2.0f / gridSize.y, 2.0f / gridSize.z);

	uint mc_numVoxels = gridSize.x * gridSize.y * gridSize.z;
	uint mc_maxVerts = gridSize.x * gridSize.y * 100u;
	uint mc_activeVoxels = 0u;
	uint mc_totalVerts = 0u;

	float mc_isoValue = 0.2f;
	float mc_deviceIsoValue = 0.005f;

	//GLuint posVbo;
	//GLuint normalVbo;

	//struct cudaGraphicsResource* cuda_posvbo_resource;
	//struct cudaGraphicsResource* cuda_normalvbo_resource;

	float4* mc_hPos;
	float4* mc_hNormal;

	float4* mc_dPos;
	float4* mc_dNormal;
	uint* mc_dVoxelVerts = 0;
	uint* mc_dVoxelVertsScan = 0;
	uint* mc_dVoxelOccupied = 0;
	uint* mc_dVoxelOccupiedScan = 0;
	uint* mc_dCompVoxelArray;

	uint* mc_numVertsTable = nullptr;
	uint* mc_edgeTable = nullptr;
	uint* mc_triTable = nullptr;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> Uv0;
	TArray<FColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	
};
