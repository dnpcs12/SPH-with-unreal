// Fill out your copyright notice in the Description page of Project Settings.


#include "MyStaticMeshActor.h"

void AMyStaticMeshActor::setVelocity(FVector vel)
{
	mVelocity = vel;
	
	UStaticMeshComponent* comp = reinterpret_cast<UStaticMeshComponent*>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
	if (comp->Mobility.GetValue() == EComponentMobility::Static)
		vel = {0,0,0};
	else
		comp->SetPhysicsLinearVelocity(vel);

}
