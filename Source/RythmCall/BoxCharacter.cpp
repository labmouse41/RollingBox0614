// Fill out your copyright notice in the Description page of Project Settings.


#include "BoxCharacter.h"
#include "Obstacle.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ArrowComponent.h"
#include "NiagaraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "InputMappingContext.h"
#include "DrawDebugHelpers.h"



// Sets default values
ABoxCharacter::ABoxCharacter()
{
	
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Cube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxCharacter"));
	Cube->SetSimulatePhysics(true);
	Cube->SetNotifyRigidBodyCollision(true);

	Cube->SetCollisionProfileName("PhysicsActor");
	Cube->OnComponentHit.AddDynamic(this, &ABoxCharacter::OnCharacterHit);
	// Set as root component
	RootComponent = Cube;


	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube")); // 경로와 메시 이름을 적절히 수정하세요
	if (CubeMeshAsset.Succeeded())
	{
		Cube->SetStaticMesh(CubeMeshAsset.Object);
	}
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// Configure character movement

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1500.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	ParticleOnJump = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	ParticleOnJump->SetupAttachment(RootComponent); // 적절한 Attachment 설정
	

	Arrows.SetNum(8);

	const TArray<FVector> ArrowLocations = {
	FVector(50.0f, 50.0f, 50.0f),
	FVector(50.0f, -50.0f, 50.0f),
	FVector(50.0f, 50.0f, -50.0f),
	FVector(50.0f, -50.0f, -50.0f),
	FVector(-50.0f, 50.0f, 50.0f),
	FVector(-50.0f, -50.0f, 50.0f),
	FVector(-50.0f, 50.0f, -50.0f),
	FVector(-50.0f, -50.0f, -50.0f)
	};
	
	for (int32 ArrowIndex = 0; ArrowIndex < 8; ArrowIndex++)
	{
		FString ArrowName = FString::Printf(TEXT("Arrow%d"), ArrowIndex);
		UArrowComponent* NewArrow = CreateDefaultSubobject<UArrowComponent>(*ArrowName);
		Arrows[ArrowIndex] = NewArrow;

		Arrows[ArrowIndex]->SetupAttachment(GetRootComponent());
		Arrows[ArrowIndex]->SetRelativeLocation(ArrowLocations[ArrowIndex]);
		Arrows[ArrowIndex]->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	}

}

float ABoxCharacter::GetNearestLength() const
{
	return NearestLength;
}

void ABoxCharacter::SetNearestLength(float NewNearestLength)
{
	NearestLength = NewNearestLength;
}
float ABoxCharacter::GetAirTime() const
{
	return AirTime;
}
void ABoxCharacter::SetAirTime(float NewAirTime)
{
	AirTime = NewAirTime;
}


//sequence node
void ABoxCharacter::ActivatePinsSequentially(const TArray<int32>& PinOrder, float DeltaSeconds)
{
	for (int32 PinIndex : PinOrder)
	{
		if (PinIndex == 0)
		{
			// 인덱스 0번 핀에 실행해야 할 함수를 호출합니다.
			MaxSpeedByHit();
		}
		else if (PinIndex == 1)
		{
			// 인덱스 1번 핀에 실행해야 할 함수를 호출합니다.
			AirTimeControl(DeltaSeconds);

		}

	}
	
}

void ABoxCharacter::PlayJumpParticlesOnPoint(const FVector& PlayLocation, USceneComponent* CheckVelocityHere)
{
	if (ParticleOnJump)
	{
		//UE_LOG(LogTemp, Warning, TEXT("PlayLocation: %s"), *PlayLocation.ToString());
		ParticleOnJump->SetRelativeLocation(PlayLocation);
		//UE_LOG(LogTemp, Warning, TEXT("CheckVelocityHere: %s"), *CheckVelocityHere->GetRelativeLocation().ToString());
		FVector Param = Cube->GetPhysicsLinearVelocityAtPoint(CheckVelocityHere->GetRelativeLocation());
		ParticleOnJump->SetVectorParameter(FName("InheritVelocity"), Param * 0.01f);
		ParticleOnJump->ResetSystem();
		SetAirTime(0.0);
		//UE_LOG(LogTemp, Warning, TEXT("Jump"));
	}
}

// Called when the game starts or when spawned
void ABoxCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	ParticleOnJump->RegisterComponent();
	
	if (ParticleOnJump)
	{
		// Niagara 컴포넌트에 사용자 지정 파라미터 설정
		ParticleOnJump->SetFloatParameter(FName("InitialForce"), InitialForce);
		ParticleOnJump->SetIntParameter(FName("SpawnNumber"), SpawnNumber);
	}


	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AObstacle::StaticClass(), AllActors);
	
	for (AActor* Actor : AllActors)
	{
		AObstacle* Obstacle = Cast<AObstacle>(Actor);
		if (Obstacle)
		{
			// AObstacle의 멤버 변수 사용
			ActorsToIgnore = Obstacle->OtherObstacles;

			// 멤버 변수 값을 로그로 출력
		}
	}
}
// Called every frame
void ABoxCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 호출하고자 하는 Sequence 함수를 여기에 작성합니다.
	TArray<int32> PinOrder;
	PinOrder.Add(0);
	PinOrder.Add(1);
	// PinOrder에 실행하고자 하는 핀의 인덱스를 추가합니다. (예: [0, 1, 2])
	ActivatePinsSequentially(PinOrder,DeltaTime);
}

// Called to bind functionality to input
void ABoxCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Moving
		//EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABoxCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABoxCharacter::Look);

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABoxCharacter::Jump);
	}

}


bool ABoxCharacter::CastLineToBottom(const FVector& OffSet)
{
	
	// Set up the trace parameters
	FCollisionQueryParams TraceParams(FName(TEXT("LineTrace")), false, this);
	TraceParams.bTraceComplex = false;
	TraceParams.AddIgnoredActor(this);

	//AActor* ActorToIgnore = nullptr;

	//// 액터를 가져오는 방법에 따라 ActorToIgnore를 설정합니다.
	//ActorToIgnore = GetWorld()->SpawnActor<AObstacle>();


	TraceParams.AddIgnoredActors(ActorsToIgnore);
	

	;
	// Set up the object query parameters
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Vehicle);
	ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Destructible);
	
	// Perform the line trace
	FHitResult HitResult;
	FVector EndPoint = FVector(OffSet.X, OffSet.Y, OffSet.Z -2.2f);

	bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, OffSet, EndPoint, ObjectParams, TraceParams);
	
	// Debug draw the line trace
	//DrawDebugLine(GetWorld(), OffSet, EndPoint, FColor::Red, , 2.0f, 0, 10.1f);
	
	return bHit;
	
}

FOnGroundReturn ABoxCharacter::OnGround()
{
	FOnGroundReturn Result; 
	Result.ReturnValue = false;
	Result.TouchingThis = Arrows[0];
	if (Cube)
	{
		for (int i = 0; i < Arrows.Num(); i++)
		{
			//일단 실행! Arrow가 있는지 확인
			try {
				if (Arrows.IsValidIndex(i) && Arrows[i] && CastLineToBottom(Arrows[i]->GetComponentLocation()))
				{
					Result.ReturnValue = true;
					Result.TouchingThis = Arrows[i];
					break;
				}
				else
				{
					Result.TouchingThis = Arrows[7];
				}
			}
			catch (const std::exception& e) {
				UE_LOG(LogTemp, Error, TEXT("Exception occurred: %s"), *FString(e.what()));
				Result.ReturnValue = false;
				break;
			}
			catch (...) {
				UE_LOG(LogTemp, Error, TEXT("Unknown exception occurred"));
				Result.ReturnValue = false;
				break;
			}
		}
	}
	return Result;
}
void ABoxCharacter::CapMaxSpeed()
{
	if (Cube)
	{
		FVector LinearVelocity = Cube->GetPhysicsLinearVelocity();
		//비교를 위해 vectorlength-> float로 변환
		float VelocityMagnitude = LinearVelocity.Size();
		MaxSpeed = 1222.0f;

		// 최대 속도를 벡터로 변환
		FVector MaxSpeedVector = FVector(MaxSpeed, MaxSpeed, MaxSpeed);

		if (VelocityMagnitude > MaxSpeed)
		{
			// 속도 벡터를 정규화한 후 최대 속도 벡터를 곱하여 새로운 속도 벡터 생성
			FVector NewLinearVelocity = MaxSpeedVector * (LinearVelocity.GetSafeNormal());

			// 선형 속도 설정 함수 호출
			Cube->SetPhysicsLinearVelocity(NewLinearVelocity);
		}
	}
}

void ABoxCharacter::Move(const FInputActionValue& Value,float Yaw)
{

	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Cube&&OnGround().ReturnValue)
	{
		FRotator RightVectorRotation(0.0f, Yaw + 90.0f, 0.0f);
		FVector RightVector = RightVectorRotation.Vector();
		//RightVector.Z = 0.0f;

		FRotator ForwardVectorRotation(0.0f, Yaw, 0.0f);
		FVector ForwardVector = ForwardVectorRotation.Vector();
		//ForwardVector.Z = 0.0f;
		Accel = 999999.0f;

		// 액터 참조

		FVector ResultVector = (RightVector * MovementVector.X) + (ForwardVector * MovementVector.Y);
		ResultVector.Normalize();

		FVector NewLinearVelocity = ResultVector * Accel * GetWorld()->GetDeltaSeconds();
				// 선형 속도 설정 함수 호출
		Cube->SetPhysicsLinearVelocity(NewLinearVelocity, true);

		CapMaxSpeed();
		
		//UE_LOG(LogTemp, Warning, TEXT("Move() - MovementVector: (%f, %f), ResultVector: (%f, %f, %f)"),
			//MovementVector.X, MovementVector.Y, ResultVector.X, ResultVector.Y, ResultVector.Z);
	}
}


void ABoxCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABoxCharacter::Jump()
{
	
	if (Cube&& OnGround().ReturnValue)
	{
		JumpForce = 777.0f;
		FVector LinearVelocity = Cube->GetPhysicsLinearVelocity();
		FVector NewLinearVelocity = FVector(LinearVelocity.X, LinearVelocity.Y, JumpForce);

		// 선형 속도 설정 함수 호출
		Cube->SetPhysicsLinearVelocity(NewLinearVelocity);

		PlayJumpParticlesOnPoint(OnGround().TouchingThis->GetRelativeLocation(), OnGround().TouchingThis);
	}
	
}
void ABoxCharacter::OnCharacterHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	SetNearestLength(9999999.0f);
	for (int32 Index = 0; Index < Arrows.Num(); Index++)
	{
		if (Arrows.IsValidIndex(Index)&& Arrows[Index]&&Index <= 7)
		{
			SetNearestPoint(Arrows[Index], Hit.Location);
		}
	}
}

FLengthSquaredFromComponentToPointReturn ABoxCharacter::LengthSquaredFromComponentToPoint(USceneComponent* InComponent, const FVector& Point)
{
	FLengthSquaredFromComponentToPointReturn ReturnValue;
	FVector ComponentLocation = InComponent->GetComponentLocation();
	FVector DistanceVector = Point - ComponentLocation;
	float SquaredDistance = DistanceVector.SizeSquared();
	
	ReturnValue.LengthSquared = SquaredDistance;
	if (InComponent)
	{
		ReturnValue.OutComponent = InComponent;
	}

	return ReturnValue;
	
}

void ABoxCharacter::SetNearestPoint(USceneComponent* InComponent, const FVector& Point)
{
	FLengthSquaredFromComponentToPointReturn Result = LengthSquaredFromComponentToPoint(InComponent, Point);
	float Init = Result.LengthSquared;

	if (Init < GetNearestLength())
	{
		SetNearestLength(Init);
		NearestPointAmong8 = Result.OutComponent;  // 포인터 할당
	}
}

void ABoxCharacter::AirTimeControl(float DeltaSeconds)
{
	FVector LinearVelocity = Cube->GetPhysicsLinearVelocity();
	float VelocityLength = LinearVelocity.Size();
	if (VelocityLength > 222)
	{
		SetAirTime(GetAirTime() + DeltaSeconds);
	}

}

void ABoxCharacter::MaxSpeedByHit()
{
	MaxSpeedByHitValue = 1888.0f;
	FVector LinearVelocity = Cube->GetPhysicsLinearVelocity();
	float VelocityLength = LinearVelocity.Size();
	if (VelocityLength > 0)
	{
		LinearVelocity.Normalize();
		FVector NewVelocity = MaxSpeedByHitValue * LinearVelocity;
		// NewVelocity를 사용하여 원하는 작업을 수행합니다.
		if (VelocityLength > MaxSpeedByHitValue)
		{
			Cube->SetPhysicsLinearVelocity(NewVelocity);
		}
	}
	

}

