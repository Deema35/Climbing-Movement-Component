// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once

#include "GameFramework/Character.h"
#include "ClimbingSystemCore.h"
#include "ClimbingCharacter.generated.h"



enum class EPointLocation : uint8
{
	Right,
	Left,
};

UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API AClimbingCharacter : public ACharacter
{
	friend class UClimbingPawnMovementComponent;

	GENERATED_BODY()
public:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly)
		class USpringArmComponent* ThridCameraSpringArm;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly)
		class USpringArmComponent* FistCameraSpringArm;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly)
		class UCameraComponent* Camera;

	UPROPERTY(Category = "ClimbingCharacter", BlueprintReadOnly)

		FVector InputDeltaVector;

	// Sets default values for this pawn's properties
	//AClimbingCharacter();
	AClimbingCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* _InputComponent) override;

	void MoveForward(float AxisValue);

	void MoveRight(float AxisValue);

	void CameraPitch(float AxisValue);

	void CameraYaw(float AxisValue);

	//UFUNCTION(BlueprintCallable, Category = "Pawn|Character")
		virtual void Jump() override;


	UFUNCTION(BlueprintCallable, Category = "ClimbingCharacter")
		void ChangeView(bool FistPirson);

	UFUNCTION(BlueprintPure, Category = "ClimbingCharacter")
		float GetAxisValueBP(FName NameAxis);

	UFUNCTION(BlueprintPure, Category = "ClimbingCharacter")
		float VectorXYAngleBP(FVector V1, FVector V2) { return VectorXYAngle(V1, V2); }

	void SwitchView();

	void CrouchFunk();

	void UnCrouchFunk();
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	void BlockCameraYawRangeFromCharacter(float LeftYawRange, float RightYawRange);

	void UnBlockCameraYawRange() { BlockYawRange = false; }

	float GetPositiveAngle(float Angle);

	EPointLocation GetLeftOrRight(float BaseAngle, float Point);

	void QuickTurn();

public:

	bool bFistPirsonView = true;

	class AOverlapObject* OverlopObject;

private:
	/** Pointer to climbing movement component*/
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class  UClimbingPawnMovementComponent* ClimbingMovement;

		float LeftYawRange;

		float RightYawRange;

		bool BlockYawRange = false;

};
