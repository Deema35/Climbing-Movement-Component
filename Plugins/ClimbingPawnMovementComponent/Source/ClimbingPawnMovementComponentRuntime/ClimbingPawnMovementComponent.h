// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "ClimbingSystemCore.h"
#include <chrono>
#include "ClimbingPawnMovementComponent.generated.h"

enum class EClimbingPawnModeType : uint8;
class FClimbingPawnModeBase;


UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API UClimbingPawnMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "ClimbingMovement")
	void SetClimbMode(EClimbingPawnModeType ClimbingMode);

	UFUNCTION(BlueprintPure, Category = "ClimbingMovement")
		EClimbingPawnModeType GetCurrentClimbingMode() const { return CurrentClimbingMode; }


	/*Height (from charecter locztion.z) where charector natch surface*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		int32 ClimbSnatchHeight = 70;

	/*Offset betvin top of climbing surfase and charecter location.z*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		int32 ClimbDeltaZ = -85;

	/*Offset betvin character and wall*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		int32 ClimbDeltaWallOffset = 45;

	/*Velocyty of climb movement*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		float ClimbVelocyty = 150;

	/*Velocyty of jump from climb state*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		float ClimbJumpVelocyty = 600;

	/*Velocyty lift on plane*/
	UPROPERTY(Category = "ClimbingMovement|LiftOnWall", EditAnywhere, BlueprintReadWrite)
		float ClimbLiftVelocyty = 500;

	/*Height (from charecter locztion.z) where jump over barier is work*/
	UPROPERTY(Category = "ClimbingMovement|JumpOverBarier", EditAnywhere, BlueprintReadWrite)
		int32 JumpOverBarierDeltaJumpHeght = 0;

	/*Minimal Velocyty lifting on barier (if character velocyty less then this)*/
	UPROPERTY(Category = "ClimbingMovement|JumpOverBarier", EditAnywhere, BlueprintReadWrite)
		int32 JumpOverBarierMinLiftVelocyty = 500;

	/*Offset from Wall when Wall Run*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		int32 WallOffset = 50;

	/*Fall Gravity Scale when charecter run on wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunFallGravityScale = 0.15;

	/*Multiplier input vector when charecter run on wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		int32 WallRunInputVelocyty = 3;

	/*Multiplier input vector when charecter run on wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		int32 WallRunCameraRoll = 20;

	/*Velocyty of jump on wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunJumpOnWallZVelocyty = 500;

	/*Velocyty of jump from wall forward*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunJumpForwardVelocyty = 600;

	/*Velocyty of jump from wall up*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunJumpUpVelocyty = 500;

	/*Offset from rope of zip line*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		int32 ZipLineDeltaZ = 0;

	/*Velocyty*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		float ZipLineVelocyty = 800;

	/*Velocyty of jump from Zip Line state*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		float ZipLineJumpVelocyty = 600;

	
	UPROPERTY(Category = "ClimbingMovement|Ladder", EditAnywhere, BlueprintReadWrite)
		float LadderMoveVelocyty = 500;

	UPROPERTY(Category = "ClimbingMovement|Ladder", EditAnywhere, BlueprintReadWrite)
		float DistnseToLadder = 20;

	/*Angle of surfase when character slide*/
	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedSlideAngle = 35;

	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedSlideVelosytyForward = 400;

	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedSlideVelosytyRight = 200;

	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedJumpVelocyty = 600;
	
	UPROPERTY(Category = "ClimbingMovement|RoundingTheCorner", EditAnywhere, BlueprintReadWrite)
		float RoundingTheCornerVelocyty = 300;

	UPROPERTY(Category = "ClimbingMovement|RoundingTheCorner", EditAnywhere, BlueprintReadWrite)
		float RoundingTheCornerRotationVelocyty = 400;

	/*Velocyty of jump near wall*/
	UPROPERTY(Category = "ClimbingMovement|WallJump", EditAnywhere, BlueprintReadWrite)
		float UnderWallJumpZVelocyty = 700;

	UPROPERTY(Category = "ClimbingMovement|WallJump", EditAnywhere, BlueprintReadWrite)
		float UnderWallJumpFromWallForwardVelocyty = 500;

	UPROPERTY(Category = "ClimbingMovement|WallJump", EditAnywhere, BlueprintReadWrite)
		float UnderWallJumpFromWallZVelocyty = 300;

	/*Distance before wall where walljunp works*/
	UPROPERTY(Category = "ClimbingMovement|WallJump", EditAnywhere, BlueprintReadWrite)
		float WallJumpDistance = 230;

	/*Velocyty of Run movement*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve RunVelocytyCurve;

	/*Velocyty of Slide movement*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve SlideVelocytyCurve;

	/*Threshold velocyty when char can Slide.*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float SlideThreshold = 0.75;

	

	
public:

	virtual void BeginPlay() override;

	/*UCharacterMovementComponent Interfase*/
	virtual bool DoJump(bool bReplayingMoves) override;

	virtual float GetMaxSpeed() const override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	inline void MoveTo(FVector Delta, FRotator NewRotation, bool CheckCollision);

	inline void YawCameraSet(int NewYaw);

	inline void AddYawCamera(int DeltaYaw);

	inline void RollCameraSet(int NewRoll);
	
	FClimbingPawnModeBase& GetMode(EClimbingPawnModeType ModeType) { return ModeStorage->Get(ModeType); }

public:

	float RunSpeedValue = 0;

	float MaxRunTime = 0;

private:

	EClimbingPawnModeType CurrentClimbingMode;
	
	std::unique_ptr<TClimbingModeStorage> ModeStorage;
};
