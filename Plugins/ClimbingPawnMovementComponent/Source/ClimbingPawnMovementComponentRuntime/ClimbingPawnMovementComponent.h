// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Curves/CurveFloat.h"
#include "ClimbingSystemCore.h"
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
		int32 ClimbDeltaZ = -50;

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
		float WallRunFallGravityScale = 0.2;

	/*Multiplier input vector when charecter run on wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		int32 WallRunInputVelocyty = 3;

	/*Velocyty of jump near wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunJumpZVelocyty = 600;

	/*Velocyty of jump from wall run state*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunJumpVelocyty = 600;

	/*Offset from rope of zip line*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		int32 ZipLineDeltaZ = -90;

	/*Velocyty*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		float ZipLineVelocyty = 800;

	/*Velocyty of jump from Zip Line state*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		float ZipLineJumpVelocyty = 600;

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

	/*Velocyty of Run movement*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve RunVelocytyCurve;

	/*Velocyty of Slide movement*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve SlideVelocytyCurve;

	/*Threshold velocyty when char can Slide.*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float SlideThreshold = 0.75;

	/*Velocyty of jump near wall*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float UnderWallJumpZVelocyty = 700;

	
public:

	/*UCharacterMovementComponent Interfase*/
	virtual bool DoJump(bool bReplayingMoves) override;

	virtual float GetMaxSpeed() const override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	inline void MoveTo(const FVector& Delta, const FRotator& NewRotation);
	
	FClimbingPawnModeBase& GetMode(EClimbingPawnModeType ModeType) { return ModeStorage.Get(ModeType); }

	void DefineRunSpeed(float DeltaTime);

	void DefineClimbMode();


private:
	EClimbingPawnModeType CurrentClimbingMode;

	EClimbingPawnModeType LastClimbingMode;
	
	float MinRunTime;
	float MaxRunTime;
	
	TClimbingModeStorage<FClimbingPawnModeBase, EClimbingPawnModeType> ModeStorage;
};
