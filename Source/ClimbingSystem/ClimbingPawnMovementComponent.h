// Copyright 2016 Dmitriy

#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "ClimbingPawnMovementComponent.generated.h"

class Lib
{
public:
	static bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, FHitResult& HitOut, EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None, ECollisionChannel CollisionChannel = ECC_Pawn, bool ReturnPhysMat = false);
	static bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, EDrawDebugTrace::Type DrawDebugType = EDrawDebugTrace::None, ECollisionChannel CollisionChannel = ECC_Pawn, bool ReturnPhysMat = false);
	static float VectorXYAngle(FVector V1, FVector V2);
	static void Msg(FVector Vector, bool OnScreen = true, bool OnLog = false);
	static void Msg(FString Message, bool OnScreen = true, bool OnLog = false);
	static void Msg(float Message, bool OnScreen = true, bool OnLog = false);

};

UENUM(BlueprintType)
enum EClimbingMode
{

	CLIMB_None		UMETA(DisplayName = "None"),

	/** Climb. */
	CLIMB_Climb	UMETA(DisplayName = "Climb"),

	/**Run on Left side wall  */
	CLIMB_LeftWallRun	UMETA(DisplayName = "Left Wall Run"),

	/** Run on Right side wall */
	CLIMB_RightWallRun	UMETA(DisplayName = "Right Wall Run"),

	/** Slide on ground */
	CLIMB_Slide	UMETA(DisplayName = "Slide"),

	CLIMB_JumpOverBarier	UMETA(DisplayName = "JumpOverBarier"),

	CLIMB_UnderWallJump UMETA(DisplayName = "UnderWallJump"),

	CLIMB_ZipLine UMETA(DisplayName = "ZipLine"),
	/*Slide from inclined surfase*/
	CLIMB_InclinedSlide UMETA(DisplayName = "InclinedSlide"),

	CLIMB_MAX	UMETA(DisplayName = "Hide")

};

UCLASS()
class CLIMBINGSYSTEM_API UClimbingPawnMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	

	UFUNCTION(BlueprintCallable, Category = "ClimbingMovement")
	void SetClimbMode(EClimbingMode _ClimbingMode);

	UFUNCTION(BlueprintPure, Category = "ClimbingMovement")
	EClimbingMode GetClimbingMode() const;

	UFUNCTION(BlueprintPure, Category = "ClimbingMovement")
	bool CanSetClimbMode(EClimbingMode ClimbingMode);

	/*Offset from top of climbing surfase*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		int32 ClimbDeltaZ;

	/*Velocyty of climb movement*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		float ClimbVelocyty;

	/*Velocyty of jump from climb state*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		float ClimbJumpVelocyty;

	/*Angle from center when state can change in degres*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunLimitAngle;

	/*Offset from Wall when Wall Run*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		int32 WallOffset;

	/*Fall Gravity Scale when charecter run on wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunFallGravityScale;

	/*Multiplier input vector when charecter run on wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		int32 WallRunInputVelocyty;

	/*Velocyty of jump near wall*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunJumpZVelocyty;

	/*Velocyty of jump from wall run state*/
	UPROPERTY(Category = "ClimbingMovement|WallRun", EditAnywhere, BlueprintReadWrite)
		float WallRunJumpVelocyty;

	/*Offset from rope of zip line*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		int32 ZipLineDeltaZ;

	/*Velocyty*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		float ZipLineVelocyty;

	/*Velocyty of jump from Zip Line state*/
	UPROPERTY(Category = "ClimbingMovement|ZipLine", EditAnywhere, BlueprintReadWrite)
		float ZipLineJumpVelocyty;

	/*Angle of surfase when character slide*/
	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedSlideAngle;

	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedSlideVelosytyForward;

	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedSlideVelosytyRight;

	UPROPERTY(Category = "ClimbingMovement|InclinedSlide", EditAnywhere, BlueprintReadWrite)
		float InclinedJumpVelocyty;

	/*Velocyty of Run movement*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float RunSpeed;

	/*delay before Run movement in sec*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float RunDelay;

	/*Velocyty of jump near wall*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float UnderWallJumpZVelocyty;

	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve SlideVelocytyCurve;


	/*UCharacterMovementComponent Interfase*/
	virtual bool DoJump(bool bReplayingMoves) override;
	virtual float GetMaxSpeed() const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

private:
	EClimbingMode ClimbingMode;
	EClimbingMode LastClimbingMode;
	bool BlockClimb;
	bool BlockWallRun;
	bool BlockInclinedSlide;
	FTimerHandle RunTimerHandle;
	bool bIsRun;
	float MinSlideTime;
	float MaxSlideTime;
	FTimerHandle InclinedSlideTimerHandle;
	
	void SetRun();
	void DefineClimbMode();
	bool SetMode(EClimbingMode ClimbingMode);
	void UnSetMode(EClimbingMode ClimbingMode);
	void UnBlockInclinedSlide();
	void UnblockClimbState();
	void UnblockWallRunState();
	bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation); //Check climb is possibly from Approximate coordinate and return realy coordinate
	bool CheckDeltaVectorInCurrentState(FVector& CheckDeltaVector, FRotator& CheckRotation); //Check climb is possibly in current character location coordinate and return realy coordinate
	bool CheckDeltaVectorInCurrentState();//Check climb is possibly in current character location without return new coordinates
	void MoveTo(const FVector& Delta, const FRotator& NewRotation);
	
};
