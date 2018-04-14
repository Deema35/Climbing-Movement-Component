// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once
#include "CoreMinimal.h"
#include "UObject/Class.h"  //For UENUM
#include <future>

class UClimbingPawnMovementComponent;
class FClimbingPawnModeBase;
class AZipLine;
class ALadder;

UENUM(BlueprintType)
enum class EClimbingPawnModeType : uint8
{
	Run,
	Climb,
	LeftWallRun,
	RightWallRun,
	Slide,
	JumpOverBarier,
	UnderWallJump,
	ZipLine,
	InclinedSlide,
	LiftOnWall,
	RoundingTheCorner,
	LadderMove,
	end
};

FClimbingPawnModeBase* EClimbingPawnModeTypeCreate(EClimbingPawnModeType ModeType, UClimbingPawnMovementComponent& _MovementComponent);

class FClimbingPawnModeBase
{
public:
	FClimbingPawnModeBase(UClimbingPawnMovementComponent& _MovementComponent) : MovementComponent(_MovementComponent) {}

	virtual ~FClimbingPawnModeBase() {}

	virtual EClimbingPawnModeType GetType() const = 0;

	virtual bool Tick(float DeltaTime) { return true; }

	virtual void SetMode() {}

	virtual void UnSetMode() {};

	virtual bool CanSetMode() { return !bBlockState; }

	bool CheckDeltaVectorInCurrentStateSimple();

	bool CheckDeltaVectorInCurrentStateSimple(FVector& CheckDeltaVector, FRotator& CheckRotation);

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) { return true; }

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) { ReturnValue = false;  return false; }

	void BlockState(std::chrono::milliseconds BlockTime);

protected:

	UClimbingPawnMovementComponent& MovementComponent;

	bool IsStateBlock() { return bBlockState; }

	static float GetYawCharacterFromWall(FHitResult HitResult) { return HitResult.Normal.Rotation().Yaw + 180; }

private:

	static void UnblockState(FClimbingPawnModeBase* ClimbingPawnMode, std::chrono::milliseconds BlockTime);


private:

	std::future<void> UnblockStateFuture;

	std::atomic<bool> bBlockState = false;

	
};

class FClimbingPawnModeRun : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeRun(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::Run; }

	virtual bool Tick(float DeltaTime) override;

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) override;

private:

	inline void DefineClimbMode();

	inline void DefineRunSpeed(float DeltaTime);

	inline void FallingControl();

private:

};

class FClimbingPawnModeClimb : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeClimb(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::Climb; }

	virtual bool Tick(float DeltaTime ) override;

	virtual void SetMode() override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) override;
};

class FClimbingPawnModeLeftWallRun : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeLeftWallRun(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::LeftWallRun; }

	virtual bool Tick(float DeltaTime ) override;

	virtual void SetMode() override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) override;

	void SetUnsetFromJump() { bUnsetFromJump = true; }

protected:

	virtual int GetRayEndSign() const { return -1; }

	virtual int GetCharRotation() const { return 270; }

	bool ChackNeedStop();

private:

	bool bUnsetFromJump = false;
};

class FClimbingPawnModeRightWallRun : public FClimbingPawnModeLeftWallRun
{
public:
	FClimbingPawnModeRightWallRun(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeLeftWallRun(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::RightWallRun; }

protected:

	virtual int GetRayEndSign() const override { return 1; }

	virtual int GetCharRotation() const override { return 90; }

};

class FClimbingPawnModeSlide : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeSlide(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::Slide; }

	virtual bool Tick(float DeltaTime ) override;

	virtual void SetMode() override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;

private:
	float MinSlideTime;
	float MaxSlideTime;
};

class FClimbingPawnModeJumpOverBarier : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeJumpOverBarier(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::JumpOverBarier; }

	virtual bool Tick(float DeltaTime ) override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;
};

class FClimbingPawnModeUnderWallJump : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeUnderWallJump(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::UnderWallJump; }

	virtual bool CanSetMode() override;

	virtual void SetMode()  override;

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) override;

	virtual bool Tick(float DeltaTime) override;

private:

	int CharRotashen;

};


class FClimbingPawnModeZipLine : public FClimbingPawnModeBase
{
public:

	FClimbingPawnModeZipLine(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::ZipLine; }

	virtual bool Tick(float DeltaTime ) override;

	virtual void SetMode() override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) override;

private:

	AZipLine* CurrentZipLine;
};

class FClimbingPawnModeInclinedSlide : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeInclinedSlide(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::InclinedSlide; }

	virtual bool Tick(float DeltaTime ) override;

	virtual void SetMode() override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) override;
};

class FClimbingPawnModeLiftOnWall : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeLiftOnWall(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::LiftOnWall; }

	virtual void UnSetMode() override;

	virtual void SetMode() override;

	virtual bool CanSetMode() override;

};

enum class ERoundingCornerState : uint8
{
	FistMove,
	Rotate,
	SecondMove
};

enum class ERoundingCornerDirection
{
	Right,
	Left
};

struct FRoundingTheCornerData
{
	ERoundingCornerState State;
	ERoundingCornerDirection MovementDirection; //True - right false - left
	float TraceLineZ;
	float RotateAngle;
};

class FClimbingPawnModeRoundingTheCorner : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeRoundingTheCorner(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::RoundingTheCorner; }

	virtual bool Tick(float DeltaTime ) override;

	virtual void SetMode() override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;

private:

	FRoundingTheCornerData RoundingTheCornerData;
};

class FClimbingPawnModeLadderMove : public FClimbingPawnModeBase
{
public:
	FClimbingPawnModeLadderMove(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent) {}

	virtual EClimbingPawnModeType GetType() const override { return EClimbingPawnModeType::LadderMove; }

	virtual bool Tick(float DeltaTime) override;

	virtual void SetMode() override;

	virtual void UnSetMode() override;

	virtual bool CanSetMode() override;

	virtual bool CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation) override;

	virtual bool DoJump(bool bReplayingMoves, bool& ReturnValue) override;

	void BlockUnSet(std::chrono::milliseconds BlockTime);
private:

	bool IsUnSetBlock() { return bBlockUnset; }

	static void UnBlockUnSet(FClimbingPawnModeLadderMove* ClimbingPawnMode, std::chrono::milliseconds BlockTime);

private:

	ALadder* CurrentLadder;

	float CurrentCharacterPosition = 0;

	std::future<void> UsetStateFuture;

	std::atomic<bool> bBlockUnset = false;

	int DeltaRotate = 0;

	int DeltaOffset = 0;
};