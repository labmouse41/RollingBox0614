// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "WavyFloor.generated.h"

UCLASS()
class RYTHMCALL_API AWavyFloor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	AWavyFloor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Root")
	class USceneComponent* WavyRootComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Map")
	class USphereComponent* NeighborDetector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, category = "Map")
	class UStaticMeshComponent* StaticMesh;

	UFUNCTION(BlueprintCallable)
	void Trigger(bool bDelay, float DelayTime);

	UPROPERTY(EditAnywhere,BlueprintReadWrite,category = "WavyFloor")
	bool bTriggered;

	UPROPERTY()
	float BeatLength;

	UPROPERTY()
	FVector AddedColor;

	UPROPERTY()
	float Glow;

	UPROPERTY()
	TArray <AWavyFloor*> NeighborList;

	UPROPERTY()
	TArray<AActor*> OverlappingActors;
	
	UPROPERTY()
	FLinearColor ColorImpact1;

	UPROPERTY()
	FLinearColor ColorImpact2;

	UPROPERTY()
	FTimerHandle TimerHandle_Delay;

	UPROPERTY()
	FTimerHandle TimerHandle;

	UFUNCTION()
	void DelayFunction(float DelayTime);

	UFUNCTION()
	void SetScalarParameterValueOnMaterials(UStaticMeshComponent* MeshComponent, FName ParameterName, float ParameterValue);

	// 서버에서 실행되는 커스텀 이벤트
	UFUNCTION(Server,BlueprintCallable,Reliable, WithValidation)
	void ServerCustomEventTrigger(bool bDelay, float DelayTime);

	// 모두에서 실행되는 커스텀 이벤트
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCustomEventTrigger(bool bDelay, float DelayTime);

	UPROPERTY(EditAnywhere)
	class UCurveFloat* TimelineCurve;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void Initialize();

	float Delay = 0.0f;

	FTimeline TimeLine;

	UFUNCTION()
	void TimeLineUpdateFunc(float Output);

	UFUNCTION()
	void TimeLineFinishFunc();

	UPROPERTY()
	float ZOffSet;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
