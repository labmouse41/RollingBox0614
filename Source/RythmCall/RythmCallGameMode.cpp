// Copyright Epic Games, Inc. All Rights Reserved.

#include "RythmCallGameMode.h"
#include "RythmCallCharacter.h"
#include "UObject/ConstructorHelpers.h"

ARythmCallGameMode::ARythmCallGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
