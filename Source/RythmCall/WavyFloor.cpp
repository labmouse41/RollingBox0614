#include "WavyFloor.h"
#include "Engine/World.h"
#include "Components/TimelineComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h" 
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Net/UnrealNetwork.h"

AWavyFloor::AWavyFloor()
{
	PrimaryActorTick.bCanEverTick = true;

	WavyRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = WavyRootComponent;

	NeighborDetector = CreateDefaultSubobject<USphereComponent>(TEXT("Checker"));
	NeighborDetector->InitSphereRadius(122.0f);
	NeighborDetector->SetupAttachment(RootComponent);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Block"));
	StaticMesh->SetupAttachment(RootComponent);

	bTriggered = false;
}

void AWavyFloor::BeginPlay()
{
	Super::BeginPlay();

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUFunction(this, FName("Initialize"));
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Delay, false);

	if (TimelineCurve != nullptr)
	{
		TimeLineUpdateDelegate.BindUFunction(this, FName("TimeLineUpdateFunc"));
		TimeLine.AddInterpFloat(TimelineCurve, TimeLineUpdateDelegate);

		TimeLineFinishDelegate.BindUFunction(this, FName("TimeLineFinishFunc"));
		TimeLine.SetTimelineFinishedFunc(TimeLineFinishDelegate);

		TimeLine.SetPlayRate(1.0f);
	}
}
void AWavyFloor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// NeighborList를 리플리케이트할 것임을 설정
	DOREPLIFETIME(AWavyFloor, NeighborList);
}
void AWavyFloor::ServerCustomEventTrigger_Implementation(bool bDelay, float DelayTime)
{
	if (!bTriggered)
	{
		bTriggered = true;

		if (bDelay)
		{
			DelayFunction(DelayTime);
		}
		else
		{
			AfterTrigger();
		}

	}
}
bool AWavyFloor::ServerCustomEventTrigger_Validate(bool bDelay, float DelayTime)
{
	return true;
}
void AWavyFloor::DelayFunction(float DelayTime)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.SetTimer(TimerHandle_Delay, this, &AWavyFloor::AfterTrigger, DelayTime, false);
	}
}

void AWavyFloor::AfterTrigger()
{
	//TimeLine.PlayFromStart();
	MulticastCustomEventAfterTrigger();
	TimeLine.PlayFromStart();

	for (const auto& Element : NeighborList)
	{
		Delay = FMath::RandRange(0.01f, 0.1f);
		Element->Trigger(true, Delay);
	}
}

void AWavyFloor::MulticastCustomEventAfterTrigger_Implementation()
{
	TimeLine.PlayFromStart();

	/*for (const auto& Element : NeighborList)
	{
		Delay = FMath::RandRange(0.01f, 0.1f);
		Element->Trigger(true, Delay);
	}*/
}

void AWavyFloor::TimeLineUpdateFunc(float Output)
{
	float ZOffset = (1.0f - Output) * 111.0f;
	FVector NewLocation(GetActorLocation().X, GetActorLocation().Y, ZOffset);
	SetActorLocation(NewLocation);
}

void AWavyFloor::TimeLineFinishFunc()
{
	bTriggered = false;
}

void AWavyFloor::Initialize()
{
	NeighborDetector->GetOverlappingActors(OverlappingActors, AWavyFloor::StaticClass());

	for (const auto& Element : OverlappingActors)
	{
		AWavyFloor* WavyFloorElement = Cast<AWavyFloor>(Element);
		if (WavyFloorElement && WavyFloorElement != this)
		{
			NeighborList.Add(WavyFloorElement);
		}
	}

	SetScalarParameterValueOnMaterials(StaticMesh, FName("DelayTime"), 2.0f);
}

void AWavyFloor::SetScalarParameterValueOnMaterials(UStaticMeshComponent* MeshComponent, FName ParameterName, float ParameterValue)
{
	if (MeshComponent)
	{
		int32 MaterialCount = MeshComponent->GetNumMaterials();

		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			UMaterialInterface* MaterialInterface = MeshComponent->GetMaterial(MaterialIndex);

			if (UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(MaterialInterface))
			{
				MaterialInstance->SetScalarParameterValue(ParameterName, ParameterValue);
			}
			else if (MaterialInterface)
			{
				UMaterialInstanceDynamic* DynamicMaterialInstance = MeshComponent->CreateDynamicMaterialInstance(MaterialIndex, MaterialInterface);
				if (DynamicMaterialInstance)
				{
					DynamicMaterialInstance->SetScalarParameterValue(ParameterName, ParameterValue);
				}
			}
		}
	}
}

void AWavyFloor::Trigger(bool bDelay, float DelayTime)
{
	ServerCustomEventTrigger(bDelay, DelayTime);
}

void AWavyFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeLine.TickTimeline(DeltaTime);
}
