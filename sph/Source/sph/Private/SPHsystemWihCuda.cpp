// Fill out your copyright notice in the Description page of Project Settings.


#include "SPHsystemWihCuda.h"

// Sets default values
ASPHsystemWihCuda::ASPHsystemWihCuda()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	particles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("particles"));
	bparticles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("bparticles"));
	ParticleProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("ParticleProceduralMeshComponent");

	ParticleProceduralMeshComponent->bUseAsyncCooking = true;
}


void ASPHsystemWihCuda::AddRigidySphere(float r, FVector initpos)
{
	addrigidy(initpos);
	r /= GetActorScale().X;
	SIZE_T count = static_cast<SIZE_T>(PI * r / particleRadius  );
	float seta = 2 * PI / count;
	uint bCount = 0;
	//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), initpos.X, initpos.Y, initpos.Z);
	
	//구 그리기
	for (SIZE_T z = 0; z < (count / 4) + 1; z++)
	{
		float xr = r * cos(seta * z);
		if (xr < 0) xr = 0;
		SIZE_T xCount = static_cast<SIZE_T>((PI * xr) / particleRadius  ) + 1;
		float xSeta = 2 * PI / xCount;

		for (SIZE_T x = 0; x < xCount; x++)
		{
			FVector pos = { (float)(xr * cos(x * xSeta)), (float)(xr * sin(x * xSeta)),(float)(r * sin(z * seta)) };
			//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), pos.X,pos.Y,pos.Z);
			pos += initpos * (1/scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, { pos.X ,pos.Y,pos.Z });
			bCount++;

			if (z == 0) continue;
			pos = { (float)(xr * cos(x * xSeta)), (float)(xr * sin(x * xSeta)),-(float)(r * sin(z * seta)) };
			//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), pos.X, pos.Y, pos.Z);
			pos += initpos * (1/scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, { pos.X ,pos.Y,pos.Z });
			bCount++;
		}

	}
	
	//UE_LOG(LogTemp, Log, TEXT("%d , %d "), m_maxParticles,m_totalParticles);

	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));

	computeBoundaryVolume(m_dBoundaryVolume, m_dPos, m_maxFluidParticles, m_numBoundaryParticles, bCount);

	m_numBoundaryParticles += bCount;
	m_totalParticles += bCount;
	m_numBoundarys.push_back(bCount);
}

void ASPHsystemWihCuda::AddRigidyHemisphere(float r, FVector initpos,bool up)
{

	addrigidy(initpos);
	r /= GetActorScale().X;
	SIZE_T count = static_cast<SIZE_T>(PI * r / particleRadius);
	float seta = 2 * PI / count;
	uint bCount = 0;
	//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), initpos.X, initpos.Y, initpos.Z);
	int dir = up ? 1 : -1;

	//반구 그리기
	for (SIZE_T z = 0; z < (count / 4) + 1; z++)
	{
		float xr = r * cos(seta * z);
		if (xr < 0) xr = 0;
		SIZE_T xCount = static_cast<SIZE_T>((PI * xr) / particleRadius) + 1;
		float xSeta = 2 * PI / xCount;

		for (SIZE_T x = 0; x < xCount; x++)
		{
			FVector pos = { (float)(xr * cos(x * xSeta)), (float)(xr * sin(x * xSeta)), dir * (float)(r * sin(z * seta)) };
			//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), pos.X,pos.Y,pos.Z);
			pos += initpos * (1/ scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, { pos.X ,pos.Y,pos.Z });
			bCount++;
		}

	}

	//UE_LOG(LogTemp, Log, TEXT("%d , %d "), m_maxParticles, m_totalParticles);

	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));
	
	computeBoundaryVolume(m_dBoundaryVolume, m_dPos, m_maxFluidParticles,m_numBoundaryParticles, bCount);

	m_numBoundaryParticles += bCount;
	m_totalParticles += bCount;
	m_numBoundarys.push_back(bCount);
}

void ASPHsystemWihCuda::AddRigidyCylinder(float r, int height, FVector initpos)
{
	//원기둥 그리기
	addrigidy(initpos);

	SIZE_T count = static_cast<SIZE_T>(PI * r / particleRadius);
	float seta = 2 * PI / count;
	
	uint bCount = 0;

	//기둥
	for (size_t z = 0; z < height; z++)
	{

		for (size_t x = 0; x < count; x++)
		{
			FVector pos = { (float)(r * cos(x * seta)), (float)(r * sin(x * seta)),z * particleRadius * 2 };
			pos += initpos * (1/ scaleCorrectedValue);
			//UE_LOG(LogTemp, Log, TEXT("pos : %f %f %f"), pos.X,pos.Y,pos.Z);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, { pos.X,pos.Y,pos.Z });
			bCount++;
		}

	}

	//윗판
	for (SIZE_T z = 0; z < (count / 4) + 1; z++)
	{
		float xr = r * cos(seta * z);
		if (xr < 0) xr = 0;
		SIZE_T xCount = static_cast<SIZE_T>((PI * xr) / particleRadius) + 1;
		//UE_LOG(LogTemp, Log, TEXT("Xcount : %d "), xCount);
		float xSeta = 2 * PI / xCount;

		for (SIZE_T x = 0; x < xCount; x++)
		{
			FVector pos = { (float)(xr * cos(x * xSeta)), (float)(xr * sin(x * xSeta)),height * particleRadius * 2 };
			pos += initpos *(1/ scaleCorrectedValue);
			//UE_LOG(LogTemp, Log, TEXT("pos : %f %f %f"), pos.X,pos.Y,pos.Z);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));

			addParticleToCuda(m_totalParticles + bCount, { pos.X ,pos.Y,pos.Z });
			bCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%d , %d "), m_maxParticles, m_totalParticles);

	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));


	computeBoundaryVolume(m_dBoundaryVolume, m_dPos, m_maxFluidParticles, m_numBoundaryParticles, bCount);

	m_numBoundaryParticles += bCount;
	m_totalParticles += bCount;
	m_numBoundarys.push_back(bCount);
}


void ASPHsystemWihCuda::AddRigidyCube(float x, float y, float z, FVector initpos)
{
	addrigidy(initpos);
	
	unsigned int bCount = 0;

	int cX = ceil(x / 4 * 100);
	int cY = ceil(y / 4 * 100);
	int cZ = ceil(z / 4 * 100);
	
	//UE_LOG(LogTemp, Log, TEXT("%d %d %d"), cX, cY,cZ);

	float spaceX = x / cX; 
	float spaceY = y / cY;
	float spaceZ = z / cZ;

	//큐브의 겉면에만 파티클을 만듦.

	//위, 아래
	for (int i = 0; i < cX; i++)
	{
		for (int j = 0; j < cY; j++)
		{
		
			FVector pos = { -(int)(cX/2.0f)* spaceX + i * spaceX ,-(int)(cY/2.0f) * spaceY + spaceY * j, -(int)(cZ/2.0f) * spaceZ};
			FVector counterPos = {pos.X,pos.Y,-pos.Z };
			pos += initpos * (1 / scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
			
			pos = counterPos;
			pos += initpos * (1 / scaleCorrectedValue);

			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
		}
	}

	//옆면 , x
	for (int i = 0; i < cY; i++)
	{
		for (int j = 0; j < cZ; j++)
		{

			FVector pos = { -(int)(cX / 2.0f) * spaceX ,-(int)(cY / 2.0f) * spaceY + spaceY * i, -(int)(cZ / 2.0f) * spaceZ + j * spaceZ };
			FVector counterPos = {-pos.X,pos.Y,pos.Z};
			
			pos += initpos * (1 / scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;

			pos = counterPos;
			pos += initpos * (1 / scaleCorrectedValue);

			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
		}
	}


	//옆면 , y
	for (int i = 0; i < cX; i++)
	{
		for (int j = 0; j < cZ; j++)
		{

			FVector pos = { -(int)(cX / 2.0f) * spaceX + i * spaceX ,-(int)(cY / 2.0f) * spaceY, -(int)(cZ / 2.0f) * spaceZ + j * spaceZ };
			FVector counterPos = {pos.X,-pos.Y,pos.Z };
			pos += initpos * (1 / scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
			
			pos = counterPos;
			pos += initpos * (1 / scaleCorrectedValue);

			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
		}
	}
	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));


	computeBoundaryVolume(m_dBoundaryVolume, m_dPos, m_maxFluidParticles, m_numBoundaryParticles, bCount);

	m_numBoundaryParticles += bCount;
	m_totalParticles += bCount;
	m_numBoundarys.push_back(bCount);

}

void ASPHsystemWihCuda::AddWheel(float r, FVector initpos)
{
	addrigidy(initpos);
    unsigned int bCount = 0u;
	float angle = 360 / 10;
	for (float i = 0; i < 360; i += angle)
	{
		addsubWheel(bCount, 0.25f, 0.15f, 0.03f, initpos, i, r);
		FVector rPos = {r,0,0};
		rPos = rPos.RotateAngleAxis(i,{0,1,0});
		UE_LOG(LogTemp, Log, TEXT("{%f %f %f}"), rPos.X, rPos.Y, rPos.Z);
	}

	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));

	computeBoundaryVolume(m_dBoundaryVolume, m_dPos, m_maxFluidParticles, m_numBoundaryParticles, bCount);

	m_numBoundaryParticles += bCount;
	m_totalParticles += bCount;
	m_numBoundarys.push_back(bCount);

}


void ASPHsystemWihCuda::SpawnFluidParticles(FVector initpos)
{
	if (spawnEnd)
	{
		RespawnParticles(initpos);
		return;
	}

	if (m_numFluidsParticles >= m_maxFluidParticles)
	{
		spawnEnd = true;
		return;
	}
	float posX = 0;
	float posY = 0;
	float posZ = 0;

	copyArrayFromDevice(m_hPos, m_dPos, sizeof(float) * 4 * m_maxParticles);

	for (size_t x = 0; x < spawnX; x++)
	{
		for (size_t y = 0; y < spawnY; y++)
		{
			posX = -boundary.X;
			posY = x * particleRadius * 2;
			posZ = y * particleRadius * 2;
			FVector pos = { posX, posY, posZ };
			pos += initpos * (1/ scaleCorrectedValue);
			addParticleToCuda(m_numFluidsParticles, pos,spawnInitVelocity);
			particles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			m_numFluidsParticles++;
			if (m_numFluidsParticles >= m_maxFluidParticles)
			{
				spawnEnd = true;
				UE_LOG(LogTemp, Log, TEXT("test? "));
				copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
				copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));
				return;
			}
		}
	}
	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));
}

void ASPHsystemWihCuda::ResistRigidy(AMyStaticMeshActor* rigidyActor, int type)
{	
	if(rigidyActor == nullptr) return;

	rigidyActors.Add(rigidyActor);
	FTransform actorT = rigidyActor->GetActorTransform();
	
	if(type==1)
	{
		AddRigidySphere(actorT.GetScale3D().X / 2, (actorT.GetLocation() - GetActorLocation()) / GetActorScale().X );
	}

	else if (type == 2)
	{
		AddRigidyHemisphere(actorT.GetScale3D().X , actorT.GetLocation() - GetActorLocation(), true);
	}
	else if (type == 3)
	{
		AddRigidyHemisphere(actorT.GetScale3D().X , actorT.GetLocation() - GetActorLocation(), false);

	}
	else if(type == 4)
	{
		int height = actorT.GetScale3D().Z /(particleRadius*2);
		AddRigidyCylinder(actorT.GetScale3D().X + 0.03f, 12, actorT.GetLocation() - GetActorLocation());
	}

}

void ASPHsystemWihCuda::ResetFluidPosition()
{
	uint i = 0;
	float posX = 0;
	float posY = 0;
	float posZ = 0;
	for (SIZE_T z = 0; z < sizeZ; z++)
	{
		for (SIZE_T y = 0; y < sizeY; y++)
		{
			for (SIZE_T x = 0; x < sizeX; x++)
			{
				posX = x * particleRadius * 2 - m_params.boundary.x / 2;
				posY = y * particleRadius * 2 - m_params.boundary.z / 2;
				posZ = z * particleRadius * 2 - m_params.boundary.y / 2;
				addParticleToCuda(i, {posX,posY,posZ});
				i++;
			}
		}
	}
	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));
}

void ASPHsystemWihCuda::RespawnParticles(FVector respawnPos)
{
	float posX = 0;
	float posY = 0;
	float posZ = 0;

	copyArrayFromDevice(m_hPos, m_dPos, sizeof(float) * 4 * m_maxParticles);
	
	for (size_t x = 0; x < spawnX; x++)
	{
		for (size_t y = 0; y < spawnY; y++)
		{
			posX = -boundary.X;
			posY = x * particleRadius * 2;
			posZ = y * particleRadius * 2;
			FVector pos = { posX, posY, posZ };
			pos += respawnPos * (1 / scaleCorrectedValue);
			addParticleToCuda(respawnIter,pos,spawnInitVelocity);
			respawnIter = (respawnIter + 1) % m_numFluidsParticles; 
		}
	}
	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));
}

void ASPHsystemWihCuda::OnOffSpawn()
{
	isSpawning = !isSpawning;
}

// Called when the game starts or when spawned
void ASPHsystemWihCuda::BeginPlay()
{
	Super::BeginPlay();
	initSystem();
	step();
	
}
void ASPHsystemWihCuda::tempProcess()
{	
	int idx = 0;
	float posX = 0;
	float posY = 0;
	float posZ = 0;

	for (SIZE_T z = 0; z < sizeZ; z++)
	{
		for (SIZE_T y = 0; y < sizeY; y++)
		{
			for (SIZE_T x = 0; x < sizeX; x++)
			{
				posX = x * particleRadius * 2 + m_params.boundary.x * scaleCorrectedValue;
				posY = y * particleRadius * 2 + m_params.boundary.y * scaleCorrectedValue;
				posZ = z * particleRadius * 2 + m_params.boundary.z * scaleCorrectedValue;

				FVector pos = { posX, posY, posZ };
				addParticleToCuda(idx, pos);

				//particles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100, mParticleScale));
				idx++;
			}
		}
	}
	copyArrayToDevice(m_dPos, m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));
}
void ASPHsystemWihCuda::addrigidy(FVector initpos)
{
	m_rigidyCount++;
	m_rigidyVel.push_back({ 0,0,0 });
	m_rigidyPos.push_back(FVectorToFloat3(initpos/scaleCorrectedValue));
	m_rigidyPreRotation.push_back({0,0,0,0});
	m_rigidyCurRotation.push_back({0,0,0,0});
}

float3 ASPHsystemWihCuda::FVectorToFloat3(FVector vec)
{
	return make_float3(vec.X,vec.Z,vec.Y);
}

void ASPHsystemWihCuda::addParticleToCuda(uint index, FVector pos, FVector vel)
{
	if(index >= m_maxParticles) return;
	m_hPos[index * 4] = pos.X;
	m_hPos[index * 4 + 1] = pos.Z;
	m_hPos[index * 4 + 2] = pos.Y;
	m_hPos[index * 4 + 3] = 1.0f;

	m_hVel[index * 4] = vel.X;
	m_hVel[index * 4 + 1] = vel.Z;
	m_hVel[index * 4 + 2] = vel.Y;
	m_hVel[index * 4 + 3] = 0.0f;
}

void ASPHsystemWihCuda::addsubWheel(unsigned int& bCount, float x, float y, float z, FVector initpos, float angle, float r)
{

	int cX = ceil(x / 4 * 100);
	int cY = ceil(y / 4 * 100);
	int cZ = ceil(z / 4 * 100);

	//UE_LOG(LogTemp, Log, TEXT("%d %d %d"), cX, cY,cZ);

	float spaceX = x / cX;
	float spaceY = y / cY;
	float spaceZ = z / cZ;

	//큐브의 겉면에만 파티클을 만듦.

	//위, 아래
	for (int i = 0; i < cX; i++)
	{
		for (int j = 0; j < cY; j++)
		{

			FVector pos = { -(int)(cX / 2.0f) * spaceX + i * spaceX ,-(int)(cY / 2.0f) * spaceY + spaceY * j, -(int)(cZ / 2.0f) * spaceZ };
			FVector counterPos = { pos.X,pos.Y,-pos.Z };
			pos += {r,0,0};
			pos = pos.RotateAngleAxis(angle, { 0,1,0 });
			pos += initpos * (1 / scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;

			pos = counterPos;
			pos += {r, 0, 0};
			pos = pos.RotateAngleAxis(angle, { 0,1,0 });

			pos += initpos * (1 / scaleCorrectedValue);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
		}
	}

	//옆면 , x
	for (int i = 0; i < cY; i++)
	{
		for (int j = 0; j < cZ; j++)
		{

			FVector pos = { -(int)(cX / 2.0f) * spaceX ,-(int)(cY / 2.0f) * spaceY + spaceY * i, -(int)(cZ / 2.0f) * spaceZ + j * spaceZ };
			FVector counterPos = { -pos.X,pos.Y,pos.Z };

			pos += {r, 0, 0};
			pos = pos.RotateAngleAxis(angle, { 0,1,0 });
			pos += initpos * (1 / scaleCorrectedValue);

			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;

			pos = counterPos;
			pos += {r, 0, 0};
			pos = pos.RotateAngleAxis(angle, { 0,1,0 });
			pos += initpos * (1 / scaleCorrectedValue);


			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
		}
	}


	//옆면 , y
	for (int i = 0; i < cX; i++)
	{
		for (int j = 0; j < cZ; j++)
		{

			FVector pos = { -(int)(cX / 2.0f) * spaceX + i * spaceX ,-(int)(cY / 2.0f) * spaceY, -(int)(cZ / 2.0f) * spaceZ + j * spaceZ };
			FVector counterPos = { pos.X,-pos.Y,pos.Z };

			pos += {r, 0, 0};
			pos = pos.RotateAngleAxis(angle, { 0,1,0 });
			pos += initpos * (1 / scaleCorrectedValue);

			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;

			pos = counterPos;
			pos += {r, 0, 0};
			pos = pos.RotateAngleAxis(angle, { 0,1,0 });
			pos += initpos * (1 / scaleCorrectedValue);

			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, pos);
			bCount++;
		}
	}

}

void ASPHsystemWihCuda::initSystem()
{
	m_gridSize = make_uint3(64, 64, 64);
	m_params.gridSize = m_gridSize;
	m_numGridCells = 64 * 64 * 64;
	m_params.numCells = m_numGridCells;

	UE_LOG(LogTemp, Log, TEXT("%f "), particleRadius);
	mParticleScale = FVector(particleRadius * 2, particleRadius * 2, particleRadius * 2);
	m_params.particleRadius = particleRadius;
	
	m_params.boundary = FVectorToFloat3(boundary*(1/ scaleCorrectedValue));
	//m_params.worldOrigin = make_float3(-1.0f, -1.0f, -1.0f);
	m_params.worldOrigin = FVectorToFloat3(-boundary * (1.1f / scaleCorrectedValue));//make_float3(-1.0f,-1.0f,-1.0f);
	float cellSize = m_params.particleRadius * 2.0f;//supportRadius;//m_params.particleRadius * 2.0f;
	//m_params.cellSize = make_float3(cellSize,cellSize,cellSize);
	m_params.cellSize = make_float3((boundary.X ) *2.3f / scaleCorrectedValue / 64, (boundary.Z ) * 2.3f / scaleCorrectedValue / 64, (boundary.Y ) * 2.3f / scaleCorrectedValue / 64);
	
	//두 그리드를 일치시킴.
	mc_voxelSize = m_params.cellSize;
	
	mc_isoValue = isoValue;//0.0005f;

	m_params.boundaryDamping = -elasticity; // ...

	m_params.gravity = make_float3(0.0f, -9.8f, 0.0f);
	m_params.timeStep = timeStep;
	m_params.restDensity = restDensity;
	m_params.mass = mass;
	
	m_params.boundaryMass = boudaryMass;
	
	m_params.viscosity = viscosity;
	m_params.rigidyViscosity = rigidyViscosity;
	m_params.surfaceTension = surfaceTension;
	m_params.threshold = threshold;
	m_params.gasStiffness = gasStiffness;

	m_params.H = supportRadius;
	m_params.H2 = m_params.H * m_params.H;
	m_params.Wpoly6 = 315.0f / (64.0f * myPI * pow(m_params.H, 9));
	m_params.Wspiky_Grad = 45.0f / (myPI * pow(m_params.H, 6));
	m_params.Wviscosity_Lapl = 45.0f / (myPI * pow(m_params.H, 6));
	m_params.temp = temp;
	m_params.Cs = Cs;
	//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), mParticleScale.X, mParticleScale.Y, mParticleScale.Z);
	m_maxFluidParticles = sizeX * sizeY * sizeZ;
	m_params.boundaryStartIndex = m_maxFluidParticles;
	//float rb = (boundaryR / particleRadius) * ((boundaryR / particleRadius)+1);
	m_maxParticles = m_maxFluidParticles + maxBoundaryParticle; //rb * PI * 5;

	// allocate host storage
	m_hPos = new float[m_maxParticles * 4];
	m_hVel = new float[m_maxParticles * 4];
	memset(m_hPos, 0, m_maxParticles * 4 * sizeof(float));
	memset(m_hVel, 0, m_maxParticles * 4 * sizeof(float));


	m_hCellStart = new uint[m_numGridCells];
	memset(m_hCellStart, 0, m_numGridCells * sizeof(uint));

	m_hCellEnd = new uint[m_numGridCells];
	memset(m_hCellEnd, 0, m_numGridCells * sizeof(uint));

	// allocate GPU data
	unsigned int memSize = sizeof(float) * 4 * m_maxParticles;

	allocateArray((void**)&m_dPos,memSize);
	allocateArray((void**)&m_dVel, memSize);

	unsigned int mySize = sizeof(float) * m_maxParticles;

	allocateArray((void**)&m_dMassDensity, mySize);
	allocateArray((void**)&m_dPressure, mySize);
	allocateArray((void**)&m_dBoundaryVolume, sizeof(float) * (m_maxParticles- m_maxFluidParticles));

	allocateArray((void**)&m_dSortedPos, memSize);
	allocateArray((void**)&m_dSortedVel, memSize);

	allocateArray((void**)&m_dGridParticleHash, m_maxParticles * sizeof(uint));
	allocateArray((void**)&m_dGridParticleIndex, m_maxParticles * sizeof(uint));

	allocateArray((void**)&m_dCellStart, m_numGridCells * sizeof(uint));
	allocateArray((void**)&m_dCellEnd, m_numGridCells * sizeof(uint));


	/*---------------------------------initMC------------------------------------------------*/
	mc_hPos = new float4[mc_maxVerts];
	mc_hNormal = new float4[mc_maxVerts];

	cudaMalloc((void**)&(mc_dPos), mc_maxVerts * sizeof(float) * 4);
	cudaMalloc((void**)&(mc_dNormal), mc_maxVerts * sizeof(float) * 4);
	// allocate textures
	allocateTextures(&mc_edgeTable, &mc_triTable, &mc_numVertsTable);
	// allocate device memory
	memSize = sizeof(uint) * mc_numVoxels;
	cudaMalloc((void**)&mc_dVoxelVerts, memSize);
	cudaMalloc((void**)&mc_dVoxelVertsScan, memSize);
	cudaMalloc((void**)&mc_dVoxelOccupied, memSize);
	cudaMalloc((void**)&mc_dVoxelOccupiedScan, memSize);
	cudaMalloc((void**)&mc_dCompVoxelArray, memSize);

	/*------------------------------------------------------------------------------------*/
	if (spawnMode)
	{
		tempProcess();
	}
	else 
	{
		int idx = 0;
		float posX = 0;
		float posY = 0;
		float posZ = 0;

		for (SIZE_T z = 0; z < sizeZ; z++)
		{
		for (SIZE_T y = 0; y < sizeY; y++)
		{
			for (SIZE_T x = 0; x < sizeX; x++)
			{
				posX = x * particleRadius * 2 - m_params.boundary.x/2;
				posY = y * particleRadius * 2 - m_params.boundary.z/2;
				posZ = z * particleRadius * 2 - m_params.boundary.y;
				FVector pos = { posX, posY, posZ };
				addParticleToCuda(idx,pos);
				if(!renderMarchingCube)
					particles->AddInstance(FTransform(FRotator::ZeroRotator, pos * scaleCorrectedValue, mParticleScale));
				idx++;
			}
		}
		}

		m_numFluidsParticles = idx;
	}
	



	m_numBoundaryParticles = 0;
	m_totalParticles = m_maxFluidParticles + m_numBoundaryParticles;
	//AddRigidySphere(boundaryR,{0,0,200});
	//AddWheel(0.3f,rigidyActors[0]->GetActorTransform().GetLocation() - GetActorLocation());
	//bnum = bCount;

	test = true;

	copyArrayToDevice(m_dPos,m_hPos,0, m_maxParticles *4*sizeof(float));
	copyArrayToDevice(m_dVel, m_hVel, 0, m_maxParticles * 4 * sizeof(float));

	setParameters(&m_params);
	//AddRigidyHemisphere(0.5, { 0,0,-100 }, false);
    //AddRigidyCylinder(boundaryR,17,{-40,-40,-200});
}



void ASPHsystemWihCuda::finalize()
{
	delete[] m_hPos;
	delete[] m_hVel;

	delete[] m_hCellStart;
	delete[] m_hCellEnd;

	freeArray(m_dPos);
	freeArray(m_dVel);
	// 내가 추가한거
	freeArray(m_dMassDensity);
	freeArray(m_dPressure);
	
	freeArray(m_dBoundaryVolume);

	freeArray(m_dSortedPos);
	freeArray(m_dSortedVel);

	freeArray(m_dGridParticleHash);
	freeArray(m_dGridParticleIndex);
	freeArray(m_dCellStart);
	freeArray(m_dCellEnd);

	//marchingCube
	cudaFree(mc_dPos);
	cudaFree(mc_dNormal);
	destroyAllTextureObjects();
	cudaFree(mc_edgeTable);
	cudaFree(mc_triTable);
	cudaFree(mc_dVoxelVerts);
	cudaFree(mc_dVoxelVertsScan);
	cudaFree(mc_dVoxelOccupied);
	cudaFree(mc_dVoxelOccupiedScan);
	cudaFree(mc_dCompVoxelArray);

}

void ASPHsystemWihCuda::step()
{

	//// update constants
	setParameters(&m_params);

	//// calculate grid hash
	calcHash(
		m_dGridParticleHash,
		m_dGridParticleIndex,
		m_dPos,
		m_totalParticles);

	//// sort particles based on hash
	sortParticles(m_dGridParticleHash, m_dGridParticleIndex, m_totalParticles);

	//// reorder particle arrays into sorted order and
	//// find start and end of each cell
	reorderDataAndFindCellStart(
		m_dCellStart,
		m_dCellEnd,
		m_dSortedPos,
		m_dSortedVel,
		m_dGridParticleHash,
		m_dGridParticleIndex,
		m_dPos,
		m_dVel,
		m_totalParticles,
		m_numGridCells);

	collide(
		m_dVel,
		m_dMassDensity,
		m_dPressure,
		m_dBoundaryVolume,
		m_dSortedPos,
		m_dSortedVel,
		m_dGridParticleIndex,
		m_dCellStart,
		m_dCellEnd,
		m_totalParticles,
		m_numFluidsParticles,
		m_maxFluidParticles,
		m_numGridCells,
		timeStep);

	updateRigidyPosition();

	//// integrate
	integrateSystem(
		m_dPos,
		m_dVel,
		m_rigidyCount,
		m_rigidyPos.data(),
		m_rigidyVel.data(),
		m_rigidyPreRotation.data(),
		m_rigidyCurRotation.data(),
		m_numFluidsParticles,
		m_maxFluidParticles,
		m_numBoundarys.data());
	computeIsoface();

	createIsosurface();

	copyArrayFromDevice(m_hPos, m_dPos, sizeof(float) * 4 * m_maxParticles);

	if(!renderMarchingCube)
	{
		updatePosition();
	}
		
}

void ASPHsystemWihCuda::computeIsoface()
{
	int threads = 128;
	dim3 grid(mc_numVoxels / threads, 1, 1);

	// get around maximum grid size of 65535 in each dimension
	if (grid.x > 65535)
	{
		grid.y = grid.x / 32768;
		grid.x = 32768;
	}

	// calculate number of vertices need per voxel
	launch_classifyVoxel(grid, threads,
		mc_dVoxelVerts, mc_dVoxelOccupied,
		gridSize, gridSizeShift, gridSizeMask,
		mc_numVoxels, mc_voxelSize, mc_isoValue, m_dSortedPos, m_dCellStart, m_dCellEnd,m_dGridParticleIndex);


#if SKIP_EMPTY_VOXELS
	ThrustScanWrapper(mc_dVoxelOccupiedScan, mc_dVoxelOccupied, mc_numVoxels);


	// read back values to calculate total number of non-empty voxels
	// since we are using an exclusive scan, the total is the last value of
	// the scan result plus the last value in the input array
	{
		uint lastElement, lastScanElement;
		cudaMemcpy((void*)&lastElement,
			(void*)(mc_dVoxelOccupied + mc_numVoxels - 1),
			sizeof(uint), cudaMemcpyDeviceToHost);
		cudaMemcpy((void*)&lastScanElement,
			(void*)(mc_dVoxelOccupiedScan + mc_numVoxels - 1),
			sizeof(uint), cudaMemcpyDeviceToHost);

		mc_activeVoxels = lastElement + lastScanElement;
	}

	if (mc_activeVoxels == 0)
	{
		// return if there are no full voxels
		//UE_LOG(LogTemp, Log, TEXT("marchingCube verts : %d"), mc_totalVerts);
		mc_totalVerts = 0;
		return;
	}

	// compact voxel index array
	launch_compactVoxels(grid, threads, mc_dCompVoxelArray, mc_dVoxelOccupied, mc_dVoxelOccupiedScan, mc_numVoxels);
	//getLastCudaError("compactVoxels failed");
#endif

	ThrustScanWrapper(mc_dVoxelVertsScan, mc_dVoxelVerts, mc_numVoxels);

#if DEBUG_BUFFERS
	printf("voxelVertsScan:\n");
	dumpBuffer(d_voxelVertsScan, numVoxels, sizeof(uint));
#endif

	// readback total number of vertices
	{
		uint lastElement, lastScanElement;
		cudaMemcpy((void*)&lastElement,
			(void*)(mc_dVoxelVerts + mc_numVoxels - 1),
			sizeof(uint), cudaMemcpyDeviceToHost);
		cudaMemcpy((void*)&lastScanElement,
			(void*)(mc_dVoxelVertsScan + mc_numVoxels - 1),
			sizeof(uint), cudaMemcpyDeviceToHost);

		mc_totalVerts = lastElement + lastScanElement;
	}

	// generate triangles, writing to vertex buffers
	//size_t num_bytes;
	// DEPRECATED: checkCudaErrors(cudaGLMapBufferObject((void**)&d_pos, posVbo));
	//checkCudaErrors(cudaGraphicsMapResources(1, &cuda_posvbo_resource, 0));
	//checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void**)&d_pos, &num_bytes, cuda_posvbo_resource));

	// DEPRECATED: checkCudaErrors(cudaGLMapBufferObject((void**)&d_normal, normalVbo));
	//checkCudaErrors(cudaGraphicsMapResources(1, &cuda_normalvbo_resource, 0));
	//checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void**)&d_normal, &num_bytes, cuda_normalvbo_resource));

#if SKIP_EMPTY_VOXELS
	dim3 grid2((int)ceil(mc_activeVoxels / (float)NTHREADS), 1, 1);
#else
	dim3 grid2((int)ceil(mc_numVoxels / (float)NTHREADS), 1, 1);
#endif

	while (grid2.x > 65535)
	{
		grid2.x /= 2;
		grid2.y *= 2;
	}

	launch_generateTriangles(grid2, NTHREADS, mc_dPos, mc_dNormal,
		mc_dCompVoxelArray,
		mc_dVoxelVertsScan,
		gridSize, gridSizeShift, gridSizeMask,
		mc_voxelSize, mc_isoValue, mc_activeVoxels, mc_maxVerts,
		(float4*)m_dSortedPos, m_dCellStart, m_dCellEnd, m_dGridParticleIndex);

	// DEPRECATED:      checkCudaErrors(cudaGLUnmapBufferObject(normalVbo));
	//checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_normalvbo_resource, 0));
	// DEPRECATED:      checkCudaErrors(cudaGLUnmapBufferObject(posVbo));
	//checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_posvbo_resource, 0));
	//UE_LOG(LogTemp, Log, TEXT("done computeIsoface : %d"), mc_totalVerts);
}

void ASPHsystemWihCuda::createIsosurface()
{
	ParticleProceduralMeshComponent->ClearAllMeshSections();

	cudaMemcpy(mc_hPos, mc_dPos, sizeof(float) * 4 * mc_maxVerts, cudaMemcpyDeviceToHost);
	cudaMemcpy(mc_hNormal,mc_dNormal, sizeof(float) * 4 * mc_maxVerts, cudaMemcpyDeviceToHost);

	Vertices.Empty();
	Triangles.Empty();
	Uv0.Empty();

	Vertices.Reserve(mc_maxVerts);
	Triangles.Reserve(mc_maxVerts);
	Uv0.Reserve(mc_maxVerts);

	for (uint32 i = 0; i < mc_totalVerts / 3u; ++i)
	{
		Vertices.Add(FVector(mc_hPos[3*i].x, mc_hPos[3*i].z, mc_hPos[3*i].y ) * scaleCorrectedValue);
		Vertices.Add(FVector(mc_hPos[3 * i + 1].x, mc_hPos[3 * i + 1].z, mc_hPos[3 * i + 1].y ) * scaleCorrectedValue);
		Vertices.Add(FVector(mc_hPos[3 * i + 2].x, mc_hPos[3 * i + 2].z, mc_hPos[3 * i + 2].y ) * scaleCorrectedValue);

		Triangles.Add(static_cast<int32>(3 * i + 2));
		Triangles.Add(static_cast<int32>(3 * i + 1));
		Triangles.Add(static_cast<int32>(3 * i + 0));
		
		Uv0.Add(FVector2D(-mc_hPos[3 * i].x / 2.0f, -mc_hPos[3 * i].y / 2.0f));
		Uv0.Add(FVector2D(-mc_hPos[3 * i + 1].x / 2.0f, -mc_hPos[3 * i + 1].y / 2.0f));
		Uv0.Add(FVector2D(-mc_hPos[3 * i + 2].x / 2.0f, -mc_hPos[3 * i + 2].y / 2.0f));
		
		Normals.Add(FVector(-mc_hPos[3 * i].x, -mc_hPos[3 * i].z, -mc_hPos[3 * i].y));
		Normals.Add(FVector(-mc_hPos[3 * i + 1].x, -mc_hPos[3 * i + 1].z, -mc_hPos[3 * i + 1].y));
		Normals.Add(FVector(-mc_hPos[3 * i + 2].x, -mc_hPos[3 * i + 2].z, -mc_hPos[3 * i + 2].y));
	}


	
	ParticleProceduralMeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, Uv0, VertexColors, Tangents, false);
	if (machingCubeMaterial != nullptr)
	{
		ParticleProceduralMeshComponent->SetMaterial(0,machingCubeMaterial);
	}
}

void ASPHsystemWihCuda::updateRigidyPosition()
{	
	//FVector pos = bparticles->GetRelativeLocation();

	copyArrayFromDevice(m_hVel, m_dVel, sizeof(float) * 4 * m_maxParticles);
	copyArrayFromDevice(m_hPos, m_dPos, sizeof(float) * 4 * m_maxParticles);

	size_t start = m_maxFluidParticles;
	size_t end = m_maxFluidParticles;
	for (size_t count = 0; count < m_rigidyCount; count++)
	{

		end += m_numBoundarys[count];
		float3 prePos = m_rigidyPos[count];
		//FVector preRotation = {m_rigidyRotation[count].x,m_rigidyRotation[count].z,m_rigidyRotation[count].y};

		m_rigidyPos[count] = FVectorToFloat3(rigidyActors[count]->GetActorTransform().GetLocation() - GetActorLocation())/scaleCorrectedValue / GetActorScale().X;
		m_rigidyVel[count] = (m_rigidyPos[count] - prePos)/*/ scaleCorrectedValue */ / m_params.timeStep; //FVectorToFloat3(sumVel);
		FRotator curRotation = rigidyActors[count]->GetActorRotation();
		FQuat q = curRotation.Quaternion();

		//UE_LOG(LogTemp, Log, TEXT("euler : %f %f %f"), q.Euler().X, q.Euler().Y, q.Euler().Z);
		m_rigidyPreRotation[count] = {-m_rigidyCurRotation[count].x,-m_rigidyCurRotation[count].y,-m_rigidyCurRotation[count].z,m_rigidyCurRotation[count].w};
		m_rigidyCurRotation[count] = {q.X,q.Z,q.Y,q.W};//make_float3(curRotation.Roll, curRotation.Yaw, curRotation.Pitch);
		//m_rigidyRotation[count] =  make_float3(curRotation.Roll, curRotation.Yaw,curRotation.Pitch) - m_rigidyRotation[count]; //{0,0,1.0f};
		//UE_LOG(LogTemp, Log, TEXT("cur angular : %f %f %f"), curRotation.Roll, curRotation.Pitch, curRotation.Yaw);
		//UE_LOG(LogTemp, Log, TEXT("pre - cur angular: %f %f %f"), m_rigidyRotation[count].x, m_rigidyRotation[count].y, m_rigidyRotation[count].z);


		FVector sumVel = {0,0,0};
		FVector sumTorque = {0,0,0};
		float sumDiff = 0.0f;
		for (size_t i = start; i < end; i++)
		{
			//if(count==0) break;
			FVector vel = FVector(m_hVel[i * 4], m_hVel[i * 4 + 2], m_hVel[i * 4 + 1]);
			sumVel += vel;
			FVector diff = FVector(m_hPos[i * 4] - m_rigidyPos[count].x, 
								   m_hPos[i * 4 + 2] - m_rigidyPos[count].z, 
								   m_hPos[i * 4 + 1] - m_rigidyPos[count].y);

			sumDiff += (diff.X * diff.X + diff.Y *diff.Y + diff.Z * diff.Z);
			sumTorque += FVector::CrossProduct(diff,vel);
		}
		
		sumVel /= m_numBoundarys[count];
		sumTorque /= m_numBoundarys[count];
		sumTorque /= sumDiff;
		//sumTorque = {0.01f,0.01f,0.01f};
		
		//rigidyActors[count]->setAngularVelocity(FVector{0,0.01f,0} * scaleCorrectedValue / m_params.unrealDeltaTime * m_params.timeStep);

		rigidyActors[count]->setVelocity((sumVel )*scaleCorrectedValue * GetActorScale().X / m_params.unrealDeltaTime * m_params.timeStep);
		rigidyActors[count]->setAngularVelocity(sumTorque * 100 *100 / m_params.unrealDeltaTime * m_params.timeStep);

		//UE_LOG(LogTemp, Log, TEXT("angular1: %f %f %f"), m_rigidyRotation[count].x, m_rigidyRotation[count].y, m_rigidyRotation[count].z);

		//rigidyActors[count]->setAngularVelocity(sumTorque * scaleCorrectedValue  / m_params.unrealDeltaTime * m_params.timeStep);
		//m_rigidyVel[count] = FVectorToFloat3(rigidyActors[count]->GetVelocity()/100 * timeC /m_params.timeStep);
		//FVector diff = (m_rigidyPos[count] - prePos) / scaleCorrectedValue/ timeStep - sumVel;
		// UE_LOG(LogTemp, Log, TEXT("diff %d : %f %f %f"), count, diff.X, diff.Y ,diff.Z);
		 //UE_LOG(LogTemp, Log, TEXT("angluar velocity %d : %f %f %f"), count,m_rigidyVel[count].x, m_rigidyVel[count].z, m_rigidyVel[count].y);
		UE_LOG(LogTemp, Log, TEXT("angluar a %d : %f %f %f"), count, sumTorque.X, sumTorque.Y, sumTorque.Z);
		start = end;

	}
	//pos += sumVel * timeStep * 100;
	//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), sumVel.X,sumVel.Y,sumVel.Z);

	//bparticles->SetRelativeLocation(pos);
	
}

void ASPHsystemWihCuda::updatePosition()
{
	FVector pos;
	copyArrayFromDevice(m_hPos, m_dPos, sizeof(float) * 4 * m_maxParticles);
	for (size_t i = 0; i < m_numFluidsParticles; i++)
	{
		pos = FVector(m_hPos[i*4] * scaleCorrectedValue, m_hPos[i * 4+2]*scaleCorrectedValue,m_hPos[i * 4 + 1]* scaleCorrectedValue);
		particles->UpdateInstanceTransform(i, FTransform(FRotator::ZeroRotator,pos, mParticleScale));
	}

	int last = m_numFluidsParticles - 1;
	pos = FVector(m_hPos[last * 4]* scaleCorrectedValue, m_hPos[last * 4 + 2]* scaleCorrectedValue, m_hPos[last * 4 + 1]* scaleCorrectedValue);
	particles->UpdateInstanceTransform(last, FTransform(FRotator::ZeroRotator, pos, mParticleScale), false, true);

	for (size_t i = m_maxFluidParticles; i < m_totalParticles; i++)
	{
		pos = FVector(m_hPos[i * 4] * scaleCorrectedValue, m_hPos[i * 4 + 2] * scaleCorrectedValue, m_hPos[i * 4 + 1] * scaleCorrectedValue);
		bparticles->UpdateInstanceTransform(i- m_maxFluidParticles, FTransform(FRotator::ZeroRotator, pos, mParticleScale));
	}

	last = m_totalParticles - 1;
	pos = FVector(m_hPos[last * 4] * scaleCorrectedValue, m_hPos[last * 4 + 2] * scaleCorrectedValue, m_hPos[last * 4 + 1] * scaleCorrectedValue);
	bparticles->UpdateInstanceTransform(last- m_maxFluidParticles, FTransform(FRotator::ZeroRotator, pos, mParticleScale), false, true);


}



ASPHsystemWihCuda::~ASPHsystemWihCuda()
{
	finalize();
}


// Called every frame
void ASPHsystemWihCuda::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	timeC += DeltaTime;
	spawnTimeChecker += DeltaTime;
	m_params.unrealDeltaTime = DeltaTime;
	
	if (timeC < timeStep) return;	
	timeC = 0.0f;
	step();

	if (spawnTimeChecker > spawnTime)
	{
		if (isSpawning) SpawnFluidParticles({ 20,0,200 });
		spawnTimeChecker = 0;
	}

}

