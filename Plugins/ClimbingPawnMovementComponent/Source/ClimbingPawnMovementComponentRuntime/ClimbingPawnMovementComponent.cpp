// Copyright 2016 - 2018 Dmitriy Pavlov


#include "ClimbingPawnMovementComponent.h"
#include "ClimbingCharacter.h"
#include "OverlapObject.h"
#include "Components/SplineComponent.h"
#include "LogCategory.h"
#include "ClimbingPawnMode.h"
#include <vector>

UClimbingPawnMovementComponent::UClimbingPawnMovementComponent(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
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

	std::vector<EClimbingPawnModeType> ClimbModeList;

	for (int i = 0; i < (int)EClimbingPawnModeType::end; i++)
	{
		ClimbModeList.push_back((EClimbingPawnModeType)i);
	}

	auto ComponentGenerator = [&](EClimbingPawnModeType ModeType)
	{
		return EClimbingPawnModeTypeCreate(ModeType, *this);
	};

	ModeStorage.IniciateComponents(ClimbModeList, ComponentGenerator);
}



void  UClimbingPawnMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}
	

	if (ModeStorage.Get(CurrentClimbingMode).Tick(DeltaTime))
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}

};

void UClimbingPawnMovementComponent::DefineClimbMode()
{

	if (IsFalling())
	{
		if (ModeStorage.Get(EClimbingPawnModeType::Climb).CanSetMode()) SetClimbMode(EClimbingPawnModeType::Climb);
		
		if (ModeStorage.Get(EClimbingPawnModeType::JumpOverBarier).CanSetMode()) SetClimbMode(EClimbingPawnModeType::JumpOverBarier);

		if (ModeStorage.Get(EClimbingPawnModeType::LeftWallRun).CanSetMode()) SetClimbMode(EClimbingPawnModeType::LeftWallRun);

		if (ModeStorage.Get(EClimbingPawnModeType::RightWallRun).CanSetMode()) SetClimbMode(EClimbingPawnModeType::RightWallRun);
	}
	else
	{
		if (ModeStorage.Get(EClimbingPawnModeType::InclinedSlide).CanSetMode()) SetClimbMode(EClimbingPawnModeType::InclinedSlide);
	}

}

void UClimbingPawnMovementComponent::DefineRunSpeed(float DeltaTime)
{
	
	if (PawnOwner->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 && !Cast<AClimbingCharacter>(PawnOwner)->bIsCrouched && !IsFalling() && MinRunTime < MaxRunTime)
	{
		MinRunTime += DeltaTime;
		
	}

	float XYVelocyty = pow(pow(Velocity.X, 2) + pow(Velocity.Y, 2), 0.5);
	float MaxRunVelocyty;
	float MinRunVelocyty;

	RunVelocytyCurve.GetRichCurve()->GetValueRange(MinRunVelocyty, MaxRunVelocyty);

	if (XYVelocyty < MinRunVelocyty - 100 || Cast<AClimbingCharacter>(PawnOwner)->bIsCrouched) //This mean character stop
	{
		RunVelocytyCurve.GetRichCurve()->GetTimeRange(MinRunTime, MaxRunTime);
	}	
}

void UClimbingPawnMovementComponent::SetClimbMode(EClimbingPawnModeType ClimbingMode)
{
	
	if (CurrentClimbingMode == ClimbingMode) return;
	

	LastClimbingMode = CurrentClimbingMode;
	CurrentClimbingMode = ClimbingMode;

	ModeStorage.Get(LastClimbingMode).UnSetMode();

	if (!ModeStorage.Get(ClimbingMode).SetMode())
	{
		ModeStorage.Get(LastClimbingMode).SetMode();
		ClimbingMode = LastClimbingMode;
		UE_LOG(ClimbingPawnMovementComponentRuntime, Warning, TEXT("Cannot change mode"));
		return;
	}
	
}


bool UClimbingPawnMovementComponent::DoJump(bool bReplayingMoves)
{
	bool ReturnValue;

	if (ModeStorage.Get(CurrentClimbingMode).DoJump(bReplayingMoves, ReturnValue))
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
	
	
	if (MovementMode == EMovementMode::MOVE_Walking || MovementMode == EMovementMode::MOVE_NavWalking || MovementMode == EMovementMode::MOVE_Falling)
	{
		MaxSpeed = RunVelocytyCurve.GetRichCurveConst()->Eval(MinRunTime);
	}
	else
	{
		MaxSpeed = Super::GetMaxSpeed();
	}

	
	return MaxSpeed;
}

void UClimbingPawnMovementComponent::MoveTo(const FVector& Delta, const FRotator& NewRotation)
{
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, NewRotation, true, Hit);
	if (Hit.IsValidBlockingHit())
	{
		SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, false);
	}
}