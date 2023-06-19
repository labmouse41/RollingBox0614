// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RythmCallGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class RYTHMCALL_API ARythmCallGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	UFUNCTION()
	void CountDown();

	UPROPERTY(BlueprintReadOnly)
	int32 Minutes = 2;

	UPROPERTY(BlueprintReadOnly)
	int32 Seconds = 0;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
