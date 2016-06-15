// Copyright 2016 Dmitriy

#pragma once

#include "GameFramework/Pawn.h"

#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "ClimbingCharacter.generated.h"



UCLASS()
class CLIMBINGSYSTEM_API AClimbingCharacter : public ACharacter
{
	GENERATED_BODY()
public:

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly)
		USpringArmComponent* CameraSpringArm;
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly)
		UCameraComponent* Camera;

	// Sets default values for this pawn's properties
	//AClimbingCharacter();
	AClimbingCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void CameraPitch(float AxisValue);
	void CameraYaw(float AxisValue);

	UFUNCTION(BlueprintCallable, Category = "Pawn|Character")
		virtual void Jump() override;


	UFUNCTION(BlueprintCallable, Category = "ClimbingCharacter")
		void ChangeView(bool FistPirson);

	void SwitchView();

	void CrouchFunk();
	void UnCrouchFunk();
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	
private:
	/** Pointer to climbing movement component*/
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class  UClimbingPawnMovementComponent* ClimbingMovement;

		class AOverlapObject* OverlopObject;

		class AZipLine* ZipLine;
		
		bool bFistPirsonView;

		USkeletalMeshComponent* ClimbMesh;

		UCapsuleComponent* ClimbCapsule;

		friend class UClimbingPawnMovementComponent;

		
};
