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
	void ResistRigidy(AMyStaticMeshActor* rigidyActor);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual ~ASPHsystemWihCuda();

	void initSystem();
	void finalize();

	void step();
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
};
