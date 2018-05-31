// Copyright 2016 - 2018 Dmitriy Pavlov


#include "ClimbingPawnMovementComponent.h"
#include "ClimbingCharacter.h"
#include "OverlapObject.h"
#include "Components/SplineComponent.h"
#include "LogCategory.h"
#include "ClimbingPawnMode.h"
#include <vector>

UClimbingPawnMovementComponent::UClimbingPawnMovementComponent(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer),
ModeStorage(std::unique_ptr<TClimbingModeStorage> (new TClimbingModeStorage(*this)))
{
	MaxWalkSpeed = 500;
	JumpZVelocity = 400;
	NavAgentProps.bCanCrouch = true;
	bOrientRotationToMovement = true;

	RunVelocytyCurve.GetRichCurve()->AddKey(0, MaxWalkSpeed);
	RunVelocytyCurve.GetRichCurve()->AddKey(1.3, 1.6 * MaxWalkSpeed);
	RunVelocytyCurve.GetRichCurve()->AddKey(3, 1.8 * MaxWalkSpeed);

	float MaxRunVelocyty;
	float MinRunVelocyty;
	RunVelocytyCurve.GetRichCurve()->GetValueRange(MinRunVelocyty, MaxRunVelocyty);

	SlideVelocytyCurve.GetRichCurve()->AddKey(0, MaxRunVelocyty);
	SlideVelocytyCurve.GetRichCurve()->AddKey(0.8, 0.625 * MaxRunVelocyty);
	SlideVelocytyCurve.GetRichCurve()->AddKey(1.2, 0);


	CurrentClimbingMode = EClimbingPawnModeType::Run;
}

void UClimbingPawnMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	float MinRunTime;

	RunVelocytyCurve.GetRichCurve()->GetTimeRange(MinRunTime, MaxRunTime);
}



void  UClimbingPawnMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	
	

	if (ModeStorage->Get(CurrentClimbingMode).Tick(DeltaTime))
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}

};


void UClimbingPawnMovementComponent::SetClimbMode(EClimbingPawnModeType ClimbingMode)
{
	if (CurrentClimbingMode == ClimbingMode) return;
	ModeStorage->Get(CurrentClimbingMode).UnSetMode();

	CurrentClimbingMode = ClimbingMode;
	
	ModeStorage->Get(ClimbingMode).SetMode();
	
}


bool UClimbingPawnMovementComponent::DoJump(bool bReplayingMoves)
{
	bool ReturnValue;

	if (ModeStorage->Get(CurrentClimbingMode).DoJump(bReplayingMoves, ReturnValue))
	{
		return Super::DoJump(bReplayingMoves);
	}
	else
	{
		return ReturnValue;
	}
}


float UClimbingPawnMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed;
	
	float CurrentRunTime = MaxRunTime * RunSpeedValue;
	
	if (MovementMode == EMovementMode::MOVE_Walking || MovementMode == EMovementMode::MOVE_NavWalking || MovementMode == EMovementMode::MOVE_Falling)
	{
		MaxSpeed = RunVelocytyCurve.GetRichCurveConst()->Eval(CurrentRunTime);
	}
	else
	{
		MaxSpeed = Super::GetMaxSpeed();
	}

	return MaxSpeed;
}

void UClimbingPawnMovementComponent::MoveTo(const FVector Delta, const FRotator NewRotation, bool CheckCollision)
{
	if (CheckCollision)
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, NewRotation, true, Hit);
		if (Hit.IsValidBlockingHit())
		{
			SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, false);
		}
	}
	else
	{
		GetPawnOwner()->AddActorWorldOffset(Delta);
		GetPawnOwner()->SetActorRotation(NewRotation);
	}
}

void UClimbingPawnMovementComponent::RollCameraSet(int NewRoll)
{

	FRotator ControlRot = GetWorld()->GetFirstPlayerController()->GetControlRotation();

	ControlRot.Roll = NewRoll;

	GetWorld()->GetFirstPlayerController()->SetControlRotation(ControlRot);
}

void UClimbingPawnMovementComponent::YawCameraSet(int NewYaw)
{
	FRotator ControlRot = GetWorld()->GetFirstPlayerController()->GetControlRotation();

	ControlRot.Yaw = NewYaw;

	GetWorld()->GetFirstPlayerController()->SetControlRotation(ControlRot);
}


void UClimbingPawnMovementComponent::AddYawCamera(int DeltaYaw)
{
	FRotator ControlRot = GetWorld()->GetFirstPlayerController()->GetControlRotation();

	ControlRot.Yaw += DeltaYaw;

	GetWorld()->GetFirstPlayerController()->SetControlRotation(ControlRot);
}