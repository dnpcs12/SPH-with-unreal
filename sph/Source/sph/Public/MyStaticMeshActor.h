// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "MyStaticMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class SPH_API AMyStaticMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void setVelocity(FVector vel);

	UFUNCTION(BlueprintCallable)
		void setAngularVelocity(FVector vel);
	UFUNCTION(BlueprintCallable)
	    FVector GetAngularVelocity();
protected:
UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector mVelocity;

};
