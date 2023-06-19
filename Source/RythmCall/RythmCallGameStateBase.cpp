// Fill out your copyright notice in the Description page of Project Settings.


#include "RythmCallGameStateBase.h"

void ARythmCallGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ARythmCallGameStateBase::CountDown, 1.f, true, 0.0);

}


void ARythmCallGameStateBase::CountDown()
{
	if (Seconds != 0)
	{
		Seconds -= 1;
	}
	else 
	{
		if (Minutes == 0)
		{
			//Time Out Logic
		}
		else
		{
			Minutes -= 0;
			Seconds = 59;
		}
	}
}
