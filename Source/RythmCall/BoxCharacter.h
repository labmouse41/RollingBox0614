// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "BoxCharacter.generated.h"



USTRUCT(BlueprintType)
struct FOnGroundReturn
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	USceneComponent* TouchingThis;

	UPROPERTY(BlueprintReadWrite)
	bool ReturnValue;
};

USTRUCT(BlueprintType)
struct FLengthSquaredFromComponentToPointReturn
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float LengthSquared;

	UPROPERTY(BlueprintReadWrite)
	USceneComponent* OutComponent;
};

UCLASS()
class RYTHMCALL_API ABoxCharacter : public APawn
{
	GENERATED_BODY()

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	

public:
	// Sets default values for this pawn's properties
	ABoxCharacter();

	
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cube")
	float Accel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cube")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cube")
	float JumpForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cube")
	float MaxSpeedByHitValue;

	// NearestLength 변수의 getter 함수
	UFUNCTION(BlueprintPure, Category = "Cube")
	float GetNearestLength() const;

	// NearestLength 변수의 setter 함수
	UFUNCTION(BlueprintCallable, Category = "Cube")
	void SetNearestLength(float NewNearestLength);

	// AirTime 변수의 getter 함수
	UFUNCTION(BlueprintPure, Category = "Cube")
	float GetAirTime() const;

	// AirTime 변수의 setter 함수
	UFUNCTION(BlueprintCallable, Category = "Cube")
	void SetAirTime(float NewAirTime);


	// Cube 멤버 변수에 UStaticMeshComponent 할당
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn")
	class UStaticMeshComponent* Cube;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pawn")
	class USceneComponent* NearestPointAmong8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cube")
	TArray<class UArrowComponent*> Arrows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
	class UNiagaraComponent* ParticleOnJump;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Niagara")
	float InitialForce = 155.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	int32 SpawnNumber = 88;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	TArray<class AActor*> ActorsToIgnore;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Move(const FInputActionValue& Value,float Yaw);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION(Blueprintcallable)
	void Jump();

private:

	float NearestLength = 99999999999.0f;

	float AirTime = 0.0f;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION()
	void OnCharacterHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	bool CastLineToBottom(const FVector&OffSet);

	UFUNCTION(BlueprintCallable)
	FOnGroundReturn OnGround();
	
	UFUNCTION(BlueprintCallable)
	void CapMaxSpeed();

	UFUNCTION(BlueprintCallable)
	FLengthSquaredFromComponentToPointReturn LengthSquaredFromComponentToPoint(USceneComponent* InComponent, const FVector& Point);

	UFUNCTION(BlueprintCallable)
	void SetNearestPoint(USceneComponent* InComponent,const FVector& Point);

	UFUNCTION(BlueprintCallable)
	void AirTimeControl(float DeltaSeconds);

	UFUNCTION(BlueprintCallable)
	void MaxSpeedByHit();

	UFUNCTION(BlueprintCallable, Category = "Pin Sequence")
	void ActivatePinsSequentially(const TArray<int32>& PinOrder, float DeltaSeconds);

	UFUNCTION()
	void PlayJumpParticlesOnPoint(const FVector& PlayLocation, USceneComponent* CheckVelocityHere);
};
