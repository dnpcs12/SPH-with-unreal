// Fill out your copyright notice in the Description page of Project Settings.


#include "SPHsystemWihCuda.h"

// Sets default values
ASPHsystemWihCuda::ASPHsystemWihCuda()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	particles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("particles"));
	bparticles = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("bparticles"));
}


void ASPHsystemWihCuda::AddRigidySphere(float r, FVector initpos)
{
	addrigidy(initpos);

	SIZE_T count = static_cast<SIZE_T>(PI * r / particleRadius);
	float seta = 2 * PI / count;
	uint bCount = 0;
	//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), initpos.X, initpos.Y, initpos.Z);
	
	//구 그리기
	for (SIZE_T z = 0; z < (count / 4) + 1; z++)
	{
		float xr = r * cos(seta * z);
		if (xr < 0) xr = 0;
		SIZE_T xCount = static_cast<SIZE_T>((PI * xr) / particleRadius) + 1;
		float xSeta = 2 * PI / xCount;

		for (SIZE_T x = 0; x < xCount; x++)
		{
			FVector pos = { (float)(xr * cos(x * xSeta)), (float)(xr * sin(x * xSeta)),(float)(r * sin(z * seta)) };
			//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), pos.X,pos.Y,pos.Z);
			pos += initpos * 0.01f;
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100 , mParticleScale));
			addParticleToCuda(m_totalParticles + bCount, { pos.X ,pos.Y,pos.Z });
			bCount++;

			if (z == 0) continue;
			pos = { (float)(xr * cos(x * xSeta)), (float)(xr * sin(x * xSeta)),-(float)(r * sin(z * seta)) };
			//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), pos.X, pos.Y, pos.Z);
			pos += initpos * 0.01f;
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100, mParticleScale));
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
			pos += initpos * 0.01f;
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100, mParticleScale));
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
			pos += initpos * 0.01f;
			//UE_LOG(LogTemp, Log, TEXT("pos : %f %f %f"), pos.X,pos.Y,pos.Z);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100, mParticleScale));
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
			pos += initpos * 0.01f;
			//UE_LOG(LogTemp, Log, TEXT("pos : %f %f %f"), pos.X,pos.Y,pos.Z);
			bparticles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100, mParticleScale));

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

void ASPHsystemWihCuda::SpawnFluidParticles(FVector initpos)
{
	
	if (m_numFluidsParticles >= m_maxFluidParticles)
	{
		spawnEnd = true;
		return;
	}
	float posX = 0;
	float posY = 0;
	float posZ = 0;

	copyArrayFromDevice(m_hPos, m_dPos, sizeof(float) * 4 * m_maxParticles);
	for (size_t x = 0; x < 3; x++)
	{
		for (size_t y = 0; y < 3; y++)
		{
			posX = -boundary.X;
			posY = x * particleRadius * 2;
			posZ = y * particleRadius * 2;
			FVector pos = { posX, posY, posZ };
			pos += initpos * 0.01f;
			addParticleToCuda(m_numFluidsParticles, pos,{10,0,-3});
			particles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100, mParticleScale));
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

void ASPHsystemWihCuda::ResistRigidy(AMyStaticMeshActor* rigidyActor)
{	
	if(rigidyActor == nullptr) return;

	rigidyActors.Add(rigidyActor);
	FTransform actorT = rigidyActor->GetActorTransform();
	AddRigidySphere(actorT.GetScale3D().X/2, actorT.GetLocation() - GetActorLocation());
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
				posX = x * particleRadius * 2 + m_params.boundary.x * 100;
				posY = y * particleRadius * 2 + m_params.boundary.y * 100;
				posZ = z * particleRadius * 2 + m_params.boundary.z * 100;

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
	m_rigidyMovingPos.push_back({ 0,0,0 });
	m_rigidyVel.push_back({ 0,0,0 });
	m_rigidyPos.push_back(initpos);
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

void ASPHsystemWihCuda::initSystem()
{
	m_gridSize = make_uint3(64, 64, 64);
	m_params.gridSize = m_gridSize;
	m_numGridCells = 64 * 64 * 64;
	m_params.numCells = m_numGridCells;

	UE_LOG(LogTemp, Log, TEXT("%f "), particleRadius);
	mParticleScale = FVector(particleRadius * 2, particleRadius * 2, particleRadius * 2);
	m_params.particleRadius = particleRadius;
	m_params.boundary = FVectorToFloat3(boundary*0.01f);
	m_params.worldOrigin = make_float3(0.0f, 0.0f, 0.0f);
	float cellSize = supportRadius;//m_params.particleRadius * 2.2f;
	m_params.cellSize = make_float3(cellSize, cellSize, cellSize);

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

	m_params.H = cellSize;//particleRadius * 2.2f;
	m_params.H2 = m_params.H * m_params.H;
	m_params.Wpoly6 = 315.0f / (64.0f * myPI * pow(m_params.H, 9));
	m_params.Wspiky_Grad = 45.0f / (myPI * pow(m_params.H, 6));
	m_params.Wviscosity_Lapl = 45.0f / (myPI * pow(m_params.H, 6));
	m_params.temp = temp;
	m_params.Cs = Cs;
	//UE_LOG(LogTemp, Log, TEXT("%f %f %f"), mParticleScale.X, mParticleScale.Y, mParticleScale.Z);
	m_maxFluidParticles = sizeX * sizeY * sizeZ;

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
				posZ = z * particleRadius * 2 - m_params.boundary.y/2;

				FVector pos = { posX, posY, posZ };
				addParticleToCuda(idx,pos);
				
				particles->AddInstance(FTransform(FRotator::ZeroRotator, pos * 100, mParticleScale));
				idx++;
			}
		}
	}
	
	m_numFluidsParticles = idx;
	//tempProcess();
	m_numBoundaryParticles = 0;
	m_totalParticles = m_maxFluidParticles + m_numBoundaryParticles;
	//AddRigidySphere(boundaryR,{0,0,200});

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
		m_rigidyMovingPos.data(),
		m_rigidyVel.data(),
		m_numFluidsParticles,
		m_maxFluidParticles,
		m_numBoundarys.data());

	updatePosition();
	//if (!spawnEnd) SpawnFluidParticles({ 0,0,-40 });
}

void ASPHsystemWihCuda::updateRigidyPosition()
{	
	//FVector pos = bparticles->GetRelativeLocation();

	copyArrayFromDevice(m_hVel, m_dVel, sizeof(float) * 4 * m_maxParticles);
	size_t start = m_maxFluidParticles;
	size_t end = m_maxFluidParticles;
	for (size_t count = 0; count < m_rigidyCount; count++)
	{

		end += m_numBoundarys[count];
		FVector prePos = m_rigidyPos[count];

		m_rigidyPos[count] = (rigidyActors[count]->GetActorTransform().GetLocation() - GetActorLocation());

		m_rigidyMovingPos[count] = FVectorToFloat3((m_rigidyPos[count] - prePos) / 100);

		FVector sumVel = {0,0,0};

		for (size_t i = start; i < end; i++)
		{
			//if(count==0) break;
			sumVel += (FVector(m_hVel[i * 4], m_hVel[i * 4 + 2], m_hVel[i * 4 + 1]));
		}
		sumVel /= m_numBoundarys[count];

		rigidyActors[count]->setVelocity((sumVel )* 100 *1/timeC * m_params.timeStep);

		m_rigidyVel[count] = FVectorToFloat3(sumVel);
		//m_rigidyVel[count] = FVectorToFloat3(rigidyActors[count]->GetVelocity()/100 * timeC /m_params.timeStep);
		
		UE_LOG(LogTemp, Log, TEXT("%d : %f %f %f"), count,m_rigidyVel[count].x, m_rigidyVel[count].z, m_rigidyVel[count].y);
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
		pos = FVector(m_hPos[i*4] * 100, m_hPos[i * 4+2]*100, m_hPos[i * 4 + 1]*100);
		particles->UpdateInstanceTransform(i, FTransform(FRotator::ZeroRotator,pos, mParticleScale));
	}

	int last = m_numFluidsParticles - 1;
	pos = FVector(m_hPos[last * 4]*100, m_hPos[last * 4 + 2]*100, m_hPos[last * 4 + 1]*100);
	particles->UpdateInstanceTransform(last, FTransform(FRotator::ZeroRotator, pos, mParticleScale), false, true);

	for (size_t i = m_maxFluidParticles; i < m_totalParticles; i++)
	{
		pos = FVector(m_hPos[i * 4] * 100, m_hPos[i * 4 + 2] * 100, m_hPos[i * 4 + 1] * 100);
		bparticles->UpdateInstanceTransform(i- m_maxFluidParticles, FTransform(FRotator::ZeroRotator, pos, mParticleScale));
	}

	last = m_totalParticles - 1;
	pos = FVector(m_hPos[last * 4] * 100, m_hPos[last * 4 + 2] * 100, m_hPos[last * 4 + 1] * 100);
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

	timeC = DeltaTime;
	//if (timeC < timeStep) return;	
	//timeC = 0.0f;
	step();
}

