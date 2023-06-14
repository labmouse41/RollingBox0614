// Fill out your copyright notice in the Description page of Project Settings.


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

// Sets default values
AWavyFloor::AWavyFloor()
{

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    
    WavyRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = WavyRootComponent;

    NeighborDetector = CreateDefaultSubobject<USphereComponent>(TEXT("Checker"));
    NeighborDetector->InitSphereRadius(122.0f);
    NeighborDetector->SetupAttachment(RootComponent);

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Block"));
    StaticMesh->SetupAttachment(RootComponent);
}

void AWavyFloor::DelayFunction(float DelayTime)
{
    // GetWorld() 함수를 사용하여 월드 객체를 가져옵니다.
    UWorld* World = GetWorld();

    if (World)
    {
        // 타이머 매니저를 가져옵니다.
        FTimerManager& TimerManager = World->GetTimerManager();

        // 타이머를 설정하여 딜레이를 적용합니다.
        TimerManager.SetTimer(TimerHandle_Delay, DelayTime, false);
    }
}
// Called when the game starts or when spawned
void AWavyFloor::BeginPlay()
{
	Super::BeginPlay();
	// 다음 틱에 호출될 함수입니다.
    FTimerDelegate TimerDelegate;
    UE_LOG(LogTemp,Warning,TEXT("BeginPlay"))
    //Initialize();
    TimerDelegate.BindUFunction(this, FName("Initialize"));
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, Delay, false);

    if (TimelineCurve != nullptr)
    {

        FOnTimelineFloat TimeLineUpdateDelegate;
        FOnTimelineEvent TimeLineFinishDelegate;

        //Update Function 등록
        TimeLineUpdateDelegate.BindUFunction(this, FName("TimeLineUpdateFunc"));
        TimeLine.AddInterpFloat(TimelineCurve, TimeLineUpdateDelegate);

        //Finish Function 등록
        TimeLineFinishDelegate.BindUFunction(this, FName("TimeLineFinishFunc"));
        TimeLine.SetTimelineFinishedFunc(TimeLineFinishDelegate);
        
    }
}
void AWavyFloor::ServerCustomEventTrigger_Implementation(bool bDelay, float DelayTime)
{
    // 서버에서 실행되는 로직을 구현합니다.
    bDelay = false;
    bTriggered = false;
   
    if (!bTriggered)
    {
        bTriggered = true;

    }
    DelayFunction(bDelay * DelayTime);
    MulticastCustomEventTrigger(bDelay,DelayTime);

    for (const auto& Element : NeighborList)
    {
        DelayTime = FMath::RandRange(0.05, 0.1);
        Element->ServerCustomEventTrigger(bDelay, DelayTime);
    }
}

bool AWavyFloor::ServerCustomEventTrigger_Validate(bool bDelay, float DelayTime)
{
    // 서버에서 실행되는 커스텀 이벤트의 유효성을 검사합니다.
    return true;
}

void AWavyFloor::MulticastCustomEventTrigger_Implementation(bool bDelay, float DelayTime)
{
    
    TimeLine.PlayFromStart();
    // 모두에서 실행되는 로직을 구현합니다.
}

void AWavyFloor::Initialize()
{
    NeighborDetector->GetOverlappingActors(OverlappingActors, AWavyFloor::StaticClass());
    // 각 요소에 대한 작업 수행
    for (auto Element : OverlappingActors)
    {
        AWavyFloor* WavyFloorElement = static_cast<AWavyFloor*>(Element);
        NeighborList.Add(WavyFloorElement);
        NeighborList.Remove(this);
        
        //BeatLength는 2초로 임시지정
        SetScalarParameterValueOnMaterials(StaticMesh, FName("Initialize"), 2.0f);
    }
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

void AWavyFloor::TimeLineUpdateFunc(float Output)
{
    ZOffSet = (1.0f - Output) * 111.0f;
    FVector NewLocation(this->GetActorLocation().X, this->GetActorLocation().Y, ZOffSet);
    this->SetActorLocation(NewLocation);
}

void AWavyFloor::TimeLineFinishFunc()
{
    bTriggered = false;
}

// Called every frame
void AWavyFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    TimeLine.TickTimeline(DeltaTime);
}


