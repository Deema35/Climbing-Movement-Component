// Copyright 2016 Dmitriy

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Curves/CurveFloat.h"
//#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "ClimbingPawnMovementComponent.generated.h"

enum class EDrawDebugTraceType
{
	None,
	ForOneFrame,
	ForDuration,
	Persistent
};

class Lib
{
public:
	static bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, FHitResult& HitOut, EDrawDebugTraceType DrawDebugType = EDrawDebugTraceType::None, ECollisionChannel CollisionChannel = ECC_Pawn, bool ReturnPhysMat = false);
	static bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, EDrawDebugTraceType DrawDebugType = EDrawDebugTraceType::None, ECollisionChannel CollisionChannel = ECC_Pawn, bool ReturnPhysMat = false);
	static float VectorXYAngle(FVector V1, FVector V2);

};

UENUM(BlueprintType)
enum class EClimbingMode : uint8
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
	/** Lift on barer*/
	CLIMB_JumpOverBarier	UMETA(DisplayName = "JumpOverBarier"),

	CLIMB_UnderWallJump UMETA(DisplayName = "UnderWallJump"),

	CLIMB_ZipLine UMETA(DisplayName = "ZipLine"),
	/*Slide from inclined surfase*/
	CLIMB_InclinedSlide UMETA(DisplayName = "InclinedSlide"),
	/*Lift On wall from climb*/
	CLIMB_LiftOnWall UMETA(DisplayName = "InclinedSlide"),
	/*Rounding The Corner when character climbing*/
	CLIMB_RoundingTheCorner UMETA(DisplayName = "RoundingTheCorner"),

	CLIMB_MAX	UMETA(DisplayName = "Hide")

};

UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API UClimbingPawnMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

public:
	

	UFUNCTION(BlueprintCallable, Category = "ClimbingMovement")
	void SetClimbMode(EClimbingMode _ClimbingMode);

	UFUNCTION(BlueprintPure, Category = "ClimbingMovement")
	EClimbingMode GetClimbingMode() const;

	UFUNCTION(BlueprintPure, Category = "ClimbingMovement")
	bool CanSetClimbMode(EClimbingMode ClimbingMode);

	/*Height (from charecter locztion.z) where charector natch surface*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		int32 ClimbSnatchHeight;

	/*Offset betvin top of climbing surfase and charecter location.z*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		int32 ClimbDeltaZ;

	/*Velocyty of climb movement*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		float ClimbVelocyty;

	/*Velocyty of jump from climb state*/
	UPROPERTY(Category = "ClimbingMovement|Climb", EditAnywhere, BlueprintReadWrite)
		float ClimbJumpVelocyty;

	/*Velocyty lift on plane*/
	UPROPERTY(Category = "ClimbingMovement|LiftOnWall", EditAnywhere, BlueprintReadWrite)
		float ClimbLiftVelocyty;

	/*Height (from charecter locztion.z) where jump over barier is work*/
	UPROPERTY(Category = "ClimbingMovement|JumpOverBarier", EditAnywhere, BlueprintReadWrite)
		int32 JumpOverBarierDeltaJumpHeght;

	/*Minimal Velocyty lifting on barier (if character velocyty less then this)*/
	UPROPERTY(Category = "ClimbingMovement|JumpOverBarier", EditAnywhere, BlueprintReadWrite)
		int32 JumpOverBarierMinLiftVelocyty;

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
	
	UPROPERTY(Category = "ClimbingMovement|RoundingTheCorner", EditAnywhere, BlueprintReadWrite)
		float RoundingTheCornerVelocyty;

	UPROPERTY(Category = "ClimbingMovement|RoundingTheCorner", EditAnywhere, BlueprintReadWrite)
		float RoundingTheCornerRotationVelocyty;

	/*Velocyty of Run movement*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve RunVelocytyCurve;

	/*Velocyty of Slide movement*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		FRuntimeFloatCurve SlideVelocytyCurve;

	/*Threshold velocyty when char can Slide.*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float SlideThreshold;

	/*Velocyty of jump near wall*/
	UPROPERTY(Category = "ClimbingMovement", EditAnywhere, BlueprintReadWrite)
		float UnderWallJumpZVelocyty;

	

	


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
	bool BlockJumpOverBarier;

	float MinRunTime;
	float MaxRunTime;

	float MinSlideTime;
	float MaxSlideTime;

	FTimerHandle InclinedSlideTimerHandle; 
	
	void DefineRunSpeed(float DeltaTime);
	void DefineClimbMode();
	bool SetMode(EClimbingMode NewClimbingMode);
	void UnSetMode(EClimbingMode _ClimbingMode);
	void UnBlockInclinedSlide();
	void UnblockClimbState();
	void UnblockBlockJumpOverBarierState();
	void UnblockWallRunState();
	bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation); //Check climb is possibly from Approximate coordinate and return realy coordinate
	bool CheckDeltaVectorInCurrentState(FVector& CheckDeltaVector, FRotator& CheckRotation); //Check climb is possibly in current character location coordinate and return realy coordinate
	bool CheckDeltaVectorInCurrentState();//Check climb is possibly in current character location without return new coordinates
	void MoveTo(const FVector& Delta, const FRotator& NewRotation);
	float GetYawCharacterFromWall(FHitResult HitResult);
	
	enum class ERoundingTheCornerState
	{
		FistMove,
		Rotate,
		SecondMove
	};
	struct FRoundingTheCornerData
	{
		ERoundingTheCornerState State;
		bool MovementDirection; //True - right false - left
		float TraceLineZ;
		float RotateAngle;
	};

	FRoundingTheCornerData RoundingTheCornerData;
};
