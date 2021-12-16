// Copyright Epic Games, Inc. All Rights Reserved.

#include "sphGameMode.h"
#include "sphCharacter.h"
#include "UObject/ConstructorHelpers.h"

AsphGameMode::AsphGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
