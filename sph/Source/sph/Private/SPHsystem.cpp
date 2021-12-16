// Fill out your copyright notice in the Description page of Project Settings.


#include "SPHsystem.h"

// Sets default values
ASPHsystem::ASPHsystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	particles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("particles"));
}

// Called when the game starts or when spawned
void ASPHsystem::BeginPlay()
{
	Super::BeginPlay();
	InitSystem();
	makeLinkedCellTable();
	//computeDensityAndPressure();
	//computeForce();
	//updatePosition();
}

void ASPHsystem::InitSystem()
{
	p_array.AddDefaulted(SystemConfig.sizeX * SystemConfig.sizeY * SystemConfig.sizeZ);
	int idx = 0;
	for (SIZE_T x = 0; x < SystemConfig.sizeX; x++)
	{
		for (SIZE_T y = 0; y < SystemConfig.sizeY; y++)
		{
			for (SIZE_T z = 0; z < SystemConfig.sizeZ; z++)
			{
				FVector pos = { x * 200 * SystemConfig.particleRadius, y * 200 * SystemConfig.particleRadius,-SystemConfig.boundary.Z + z * 200 * SystemConfig.particleRadius };
				p_array[idx].position = pos;
				particles->AddInstance(FTransform(FRotator::ZeroRotator,pos,FVector(SystemConfig.particleRadius*2)));
				idx++;
			}
		}
	}
	c_array.AddDefaulted(p_array.Num());
}

void ASPHsystem::makeLinkedCellTable()
{
	for (size_t i = 0; i < c_array.Num(); i++)
	{  
		if(c_array[i].particles.Num() != 0) 
			//UE_LOG(LogTemp, Log, TEXT("%d num: %d"), i, c_array[i].particles.Num());
		c_array[i].particles.Empty();
	}

	for (size_t i = 0; i < p_array.Num(); i++)
	{
		int curidx = getCellIndex(getCellPosition(p_array[i].position));
		//UE_LOG(LogTemp, Log, TEXT("curidx : %d"), curidx);
		c_array[curidx].particles.Add(i);
		
	}

}

void ASPHsystem::computeDensityAndPressure()
{
	int count = 0; 
	for (size_t i = 0; i < p_array.Num(); i++)
	{
		FVector curCellPos = getCellPosition(p_array[i].position);
		//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), curCellPos.X ,curCellPos.Y,curCellPos.Z);
		float density = 0.0f;
		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				for (int z = -1; z <= 1; z++)
				{
					int nearCellIdx = getCellIndex(curCellPos + FVector(x, y, z));
					for (uint32 j : c_array[nearCellIdx].particles)
					{
						FVector dist = p_array[i].position - p_array[j].position;
						dist *= 0.01f;
						density += SystemConfig.mass * Kernel_Wpoly6(dist);
						count++;
					}
				}
			}
		}
		p_array[i].massDensity = density;
		p_array[i].pressure = SystemConfig.gasStiffness * (p_array[i].massDensity - SystemConfig.restDensity);
		//UE_LOG(LogTemp,Log,TEXT("%f %f"),p_array[i].massDensity,p_array[i].pressure);
	}
	//UE_LOG(LogTemp, Log, TEXT("count : %d"), count);
}

void ASPHsystem::computeForce()
{
	for (int i = 0; i < p_array.Num(); i++)
	{
		FVector curCellPos = getCellPosition(p_array[i].position);
		p_array[i].F_pressure = FVector::ZeroVector;
		p_array[i].F_viscosity = FVector::ZeroVector;
		p_array[i].F_surfaceTension = FVector::ZeroVector;
		float ci = 0.0f;
		FVector ni = FVector::ZeroVector;
		float ki = 0.0f;

		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				for (int z = -1; z <= 1; z++)
				{
					int nearCellIdx = getCellIndex(curCellPos + FVector(x, y, z));
					for(uint32 j : c_array[nearCellIdx].particles)
					{
						if (j == i) continue;
						FVector dist = p_array[i].position - p_array[j].position;
						dist *= 0.01f;
						float volume = SystemConfig.mass / p_array[j].massDensity;
						//float mdi2 = p_array[i].massDensity * p_array[i].massDensity;
						//float mdj2 = p_array[j].massDensity * p_array[j].massDensity;
						//p_array[i].F_pressure -= (p_array[j].pressure / mdj2) * SystemConfig.mass  * SystemConfig.mass * Kernel_WspikyGrad(dist);
						p_array[i].F_pressure -= ((p_array[i].pressure + p_array[j].pressure) / 2.0) * volume * Kernel_WspikyGrad(dist);
						p_array[i].F_viscosity += SystemConfig.viscosity * (p_array[j].velocity - p_array[i].velocity) * volume * Kerenl_WviscoLapl(dist);
						ci += volume * Kernel_Wpoly6(dist);
						ni += volume * Kernel_Wpoly6Grad(dist);
						ki -= volume * Kernel_Wpoly6Lapl(dist);
					}

				}
			}
		}
		if (ni.Size() > SystemConfig.threshold)
		{
			ki /= ni.Size();
			p_array[i].F_surfaceTension = -SystemConfig.surfaceTension * ki * ni;
		}

	}
}

void ASPHsystem::updatePosition()
{

	for (int i = 0; i < p_array.Num(); i++)
	{
		FVector a = (p_array[i].F_pressure + p_array[i].F_viscosity + p_array[i].F_surfaceTension + SystemConfig.restDensity * g)/p_array[i].massDensity;
		p_array[i].velocity += SystemConfig.timeStep * a;
		p_array[i].position += SystemConfig.timeStep * p_array[i].velocity * 10;
		//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), p_array[i].position.X,p_array[i].position.Y,p_array[i].position.Z);
		
		if (FMath::Abs(p_array[i].position.X) > SystemConfig.boundary.X - SystemConfig.particleRadius)
		{
			float d = p_array[i].position.X > 0 ? 1:-1;
			p_array[i].position.X = (SystemConfig.boundary.X - SystemConfig.particleRadius) * d;
			p_array[i].velocity.X = -p_array[i].velocity.X * SystemConfig.elasticity;
		}

		if (FMath::Abs(p_array[i].position.Y) > SystemConfig.boundary.Y - SystemConfig.particleRadius)
		{
			float d = p_array[i].position.Y > 0 ? 1 : -1;
			p_array[i].position.Y = (SystemConfig.boundary.Y - SystemConfig.particleRadius) * d;
			p_array[i].velocity.Y = -p_array[i].velocity.Y * SystemConfig.elasticity;
		}

		if (FMath::Abs(p_array[i].position.Z) > SystemConfig.boundary.Z - SystemConfig.particleRadius)
		{
			float d = p_array[i].position.Z > 0 ? 1 : -1;
			p_array[i].position.Z = (SystemConfig.boundary.Z - SystemConfig.particleRadius) * d;
			p_array[i].velocity.Z = -p_array[i].velocity.Z * SystemConfig.elasticity;
		}

		particles->UpdateInstanceTransform(i, FTransform(FRotator::ZeroRotator, p_array[i].position, FVector(SystemConfig.particleRadius * 2)));
	}
	int last = p_array.Num() - 1;
	particles->UpdateInstanceTransform(last, FTransform(FRotator::ZeroRotator, p_array[last].position, FVector(SystemConfig.particleRadius * 2)),false,true);
}

FVector ASPHsystem::getCellPosition(FVector xyz)
{
	return FVector( (int)(xyz.X / H()), (int)(xyz.Y / H()), (int)(xyz.Z / H()) );
}

int ASPHsystem::getCellIndex(FVector xyz)
{
	xyz.X *= 73856093;
	xyz.Y *= 19349663;
	xyz.Z *= 83492791;

	return (((uint32)xyz.X ^ (uint32)xyz.Y ^ (uint32)xyz.Z) % c_array.Num());
}

float ASPHsystem::Kernel_Wpoly6(FVector r)
{
	float r2 = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
	if (H2() < r2) return 0;

	float d = (H2() - r2);

	return Wpoly6() * d * d * d;

}

FVector ASPHsystem::Kernel_Wpoly6Grad(FVector r)
{
	
	float r2 = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
	if (H2() < r2) return FVector::ZeroVector;

	float d = (H2() - r2);

	return -(Wpoly6() * 6) * r * d * d;
}

float ASPHsystem::Kernel_Wpoly6Lapl(FVector r)
{
	float r2 = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
	if (H2() < r2) return 0;

	return -(Wpoly6() * 6) * (H() - r2) * (3 * H2() - 7 * r2);
}

FVector ASPHsystem::Kernel_WspikyGrad(FVector r)
{
	float r2 = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
	if (H2() < r2) return  FVector::ZeroVector;
	if (r2 == 0) r2 = 0.00001f;

	float sr = FMath::Sqrt(r2);

	//if (sr == 0) return  FVector::ZeroVector;
	float d = (H() - sr);

	return -Wspiky_Grad() * (r / sr) * d * d;
}

float ASPHsystem::Kerenl_WviscoLapl(FVector r)
{
	float r2 = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
	if (H2() < r2) return 0;

	float d = H ()- FMath::Sqrt(r2);
	return Wviscosity_Lapl() * d;
}


constexpr float ASPHsystem::H()
{
	return SystemConfig.supportRadius;
}

constexpr float ASPHsystem::H2()
{
	return SystemConfig.supportRadius * SystemConfig.supportRadius;
}


constexpr float ASPHsystem::Wpoly6()
{
	float _H = SystemConfig.supportRadius;
	return 315.0f / (64.0f * myPI * _H * _H * _H * _H * _H * _H * _H * _H * _H);
		
}

constexpr float ASPHsystem::Wspiky_Grad()
{
	float _H = SystemConfig.supportRadius;
	return 45.0f / (myPI * _H * _H * _H * _H * _H * _H);
}

constexpr float ASPHsystem::Wviscosity_Lapl()
{
	float _H = SystemConfig.supportRadius;
	return 45.0f / (myPI * _H * _H * _H * _H * _H * _H);
}


// Called every frame
void ASPHsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	timeC += DeltaTime;
	if (timeC > SystemConfig.timeStep)
	{
		timeC = 0.0f;
		makeLinkedCellTable();
		computeDensityAndPressure();
		computeForce();
		updatePosition();
	}
}

