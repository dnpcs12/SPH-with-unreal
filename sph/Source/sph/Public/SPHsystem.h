// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SPHsystem.generated.h"


USTRUCT(BlueprintType)
struct FSPHSystemConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta  = (ClampMin = 1,ClampMax = 100))
	int32 sizeX;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
	int32 sizeY;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 1, ClampMax = 100))
	int32 sizeZ;

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
	float surfaceTension;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float threshold;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.0))
	float gasStiffness;

	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.04))
	float supportRadius;

};

USTRUCT(BlueprintType)
struct FSPHParticle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float massDensity = 0.0f;

	UPROPERTY(EditAnywhere)
	float pressure = 0.0f;

	UPROPERTY(EditAnywhere)
	FVector velocity = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	FVector position = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	FVector F_pressure = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	FVector F_viscosity = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	FVector F_surfaceTension = FVector::ZeroVector;
};
struct Cell
{
	TArray<uint32> particles;
};

UCLASS()
class SPH_API ASPHsystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASPHsystem();

	float Kernel_Wpoly6(FVector r);
	FVector Kernel_Wpoly6Grad(FVector r);
	float Kernel_Wpoly6Lapl(FVector r);

	FVector Kernel_WspikyGrad(FVector r);
	float Kerenl_WviscoLapl(FVector r);

	constexpr float H();
	constexpr float H2();
	constexpr float Wpoly6();
	constexpr float Wspiky_Grad();
	constexpr float Wviscosity_Lapl();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void InitSystem();
	void makeLinkedCellTable();
	void computeDensityAndPressure();
	void computeForce();
	void updatePosition();


	FVector getCellPosition(FVector xyz);
	int getCellIndex(FVector xyz);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
		UInstancedStaticMeshComponent* particles;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SPH Config")
		FSPHSystemConfig SystemConfig;

	const float myPI = 3.141592f;
	
	UPROPERTY(EditAnywhere)
	TArray<FSPHParticle> p_array;
	TArray<Cell> c_array;
private:
	const FVector g = FVector(0, 0, -9.82f);
	float timeC = 0.0f;
};
