// Copyright 2016 - 2018 Dmitriy Pavlov
#include "ClimbingPawnMode.h"
#include "ClimbingPawnMovementComponent.h"
#include "ClimbingCharacter.h"
#include "TimerManager.h"
#include "OverlapObject.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"

//***********************************************
//EClimbingPawnModeType
//***********************************************

static_assert(EClimbingPawnModeType::end == (EClimbingPawnModeType)11, "Fix EClimbingPawnModeTypeCreate");

FClimbingPawnModeBase* EClimbingPawnModeTypeCreate(EClimbingPawnModeType ModeType, UClimbingPawnMovementComponent& MovementComponent)
{
	switch (ModeType)
	{

	case EClimbingPawnModeType::Climb:

		return new FClimbingPawnModeClimb(MovementComponent);

	case EClimbingPawnModeType::InclinedSlide:

		return new FClimbingPawnModeInclinedSlide(MovementComponent);

	case EClimbingPawnModeType::JumpOverBarier:

		return new FClimbingPawnModeJumpOverBarier(MovementComponent);

	case EClimbingPawnModeType::LeftWallRun:

		return new FClimbingPawnModeLeftWallRun(MovementComponent);

	case EClimbingPawnModeType::LiftOnWall:

		return new FClimbingPawnModeLiftOnWall(MovementComponent);

	case EClimbingPawnModeType::RightWallRun:

		return new FClimbingPawnModeRightWallRun(MovementComponent);

	case EClimbingPawnModeType::RoundingTheCorner:

		return new FClimbingPawnModeRoundingTheCorner(MovementComponent);

	case EClimbingPawnModeType::Run:

		return new FClimbingPawnModeRun(MovementComponent);

	case EClimbingPawnModeType::Slide:

		return new FClimbingPawnModeSlide(MovementComponent);

	case EClimbingPawnModeType::UnderWallJump:

		return new FClimbingPawnModeUnderWallJump(MovementComponent);

	case EClimbingPawnModeType::ZipLine:

		return new FClimbingPawnModeZipLine(MovementComponent);

	default: throw FString("Bad EClimbingPawnModeType");

	}
}

//***********************************************
//FClimbingPawnModeBase
//***********************************************

bool FClimbingPawnModeBase::CheckDeltaVectorInCurrentStateSimple(FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	return CheckDeltaVectorInCurrentState(FVector(0, 0, 0), CheckDeltaVector, CheckRotation);
}

bool FClimbingPawnModeBase::CheckDeltaVectorInCurrentStateSimple()
{
	FVector CheckDeltaVector;
	FRotator CheckRotation;

	return CheckDeltaVectorInCurrentState(FVector(0, 0, 0), CheckDeltaVector, CheckRotation);
}

void FClimbingPawnModeBase::BlockState(float BlockTime)
{
	FTimerHandle BlockTimerHandle;

	bBlockState = true;
	UnblockStateFuture = std::async(std::launch::async, UnblockState, this, BlockTime);
}

void FClimbingPawnModeBase::UnblockState(FClimbingPawnModeBase* ClimbingPawnMode, float BlockTime)
{
	int Time = BlockTime * 1000;

	std::this_thread::sleep_for(std::chrono::milliseconds(Time));

	ClimbingPawnMode->bBlockState = false;
}

//***********************************************
//FClimbingPawnModeRun
//***********************************************

bool FClimbingPawnModeRun::Tick(float DeltaTime)
{
	MovementComponent.DefineRunSpeed(DeltaTime);
	MovementComponent.DefineClimbMode();

	return true;
}

bool FClimbingPawnModeRun::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	
	if (MovementComponent.GetMode(EClimbingPawnModeType::UnderWallJump).CanSetMode())
	{
		MovementComponent.Velocity = FVector(0, 0, MovementComponent.UnderWallJumpZVelocyty);
		MovementComponent.SetMovementMode(MOVE_Falling);
	}
	else if (MovementComponent.GetMode(EClimbingPawnModeType::LeftWallRun).CanSetMode())
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::LeftWallRun);

		MovementComponent.Velocity.Z = MovementComponent.WallRunJumpOnWallZVelocyty;
		MovementComponent.SetMovementMode(MOVE_Falling);

	}
	else if (MovementComponent.GetMode(EClimbingPawnModeType::RightWallRun).CanSetMode())
	{

		MovementComponent.SetClimbMode(EClimbingPawnModeType::RightWallRun);
		MovementComponent.Velocity.Z = MovementComponent.WallRunJumpOnWallZVelocyty;
		MovementComponent.SetMovementMode(MOVE_Falling);

	}

	else return true;
	

	ReturnValue = true;

	return false;

	
}

//***********************************************
//FClimbingPawnModeClimb
//***********************************************

bool FClimbingPawnModeClimb::Tick(float DeltaTime)
{
	FVector NewPosition;
	FRotator NewRotation;

	FVector DesiredMovementThisFrame = MovementComponent.ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * MovementComponent.ClimbVelocyty;

	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		if (CheckDeltaVectorInCurrentState(DesiredMovementThisFrame, NewPosition, NewRotation))
		{
			MovementComponent.MoveTo(NewPosition, NewRotation);
		}
		else if (MovementComponent.GetMode(EClimbingPawnModeType::RoundingTheCorner).CanSetMode())
		{
			MovementComponent.SetClimbMode(EClimbingPawnModeType::RoundingTheCorner);
			
		}

	}

	return false;
}

void FClimbingPawnModeClimb::SetMode()
{
	FVector StartPosition;
	FRotator StartRotation;

	

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;
	MovementComponent.Velocity = FVector(0, 0, 0);

	CheckDeltaVectorInCurrentStateSimple(StartPosition, StartRotation);

	MovementComponent.MoveTo(StartPosition, StartRotation);

}

void FClimbingPawnModeClimb::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	BlockState(0.5);

	ClimbingChar->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;
	
}

bool FClimbingPawnModeClimb::CanSetMode()
{
	if (IsStateBlock()) return false;

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	

	FVector Start = ClimbingChar->GetActorLocation() + FVector(0, 0, 97) + ClimbingChar->GetActorForwardVector() * 60;
	FVector End = ClimbingChar->GetActorLocation() + FVector(0, 0, MovementComponent.ClimbSnatchHeight) + ClimbingChar->GetActorForwardVector() * 60;

	if (MovementComponent.IsFalling() && TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
	{
		if (!CheckDeltaVectorInCurrentStateSimple()) return false;

		return true;
	}

	else return false;
	
}

bool FClimbingPawnModeClimb::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	//Top Ray Var
	FVector Start;
	FVector End;
	FHitResult HitDataTop;

	if (ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveRight")) < 0)
	{
		//Left Top Ray
		Start = ClimbingChar->GetActorLocation() + InputDeltaVector + FVector(0, 0, 110) + ClimbingChar->GetActorForwardVector() * 60 - ClimbingChar->GetActorRightVector() * 40;
		End = Start - FVector(0, 0, 97);
		HitDataTop;

		if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitDataTop)) return false;

		//Left Ray
		Start = FVector((ClimbingChar->GetActorLocation() + InputDeltaVector).X, (ClimbingChar->GetActorLocation() + InputDeltaVector).Y, HitDataTop.Location.Z - 10) - ClimbingChar->GetActorRightVector() * 40;
		End = Start + ClimbingChar->GetActorForwardVector() * 70;

		if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End)) return false;

	}
	else
	{
		//Right Top Ray
		Start = ClimbingChar->GetActorLocation() + InputDeltaVector + FVector(0, 0, 110) + ClimbingChar->GetActorForwardVector() * 60 + ClimbingChar->GetActorRightVector() * 40;
		End = Start - FVector(0, 0, 97);
		HitDataTop;
		if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitDataTop)) return false;

		//Right Ray
		Start = FVector((ClimbingChar->GetActorLocation() + InputDeltaVector).X, (ClimbingChar->GetActorLocation() + InputDeltaVector).Y, HitDataTop.Location.Z - 10) + ClimbingChar->GetActorRightVector() * 40;
		End = Start + ClimbingChar->GetActorForwardVector() * 70;
		if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End)) return false;
	}

	//Middle Top Ray
	Start = ClimbingChar->GetActorLocation() + InputDeltaVector + FVector(0, 0, 110) + ClimbingChar->GetActorForwardVector() * 60;
	End = Start - FVector(0, 0, 97);
	HitDataTop;

	if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitDataTop)) return false;

	//Middel Ray
	FHitResult HitDataMiddel;
	Start = FVector((ClimbingChar->GetActorLocation() + InputDeltaVector).X, (ClimbingChar->GetActorLocation() + InputDeltaVector).Y, HitDataTop.Location.Z - 10);
	End = Start + ClimbingChar->GetActorForwardVector() * 60;

	if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitDataMiddel)) return false;

	FVector CharLoc = HitDataMiddel.Location;
	CharLoc.Z = HitDataTop.Location.Z + MovementComponent.ClimbDeltaZ;

	CheckRotation.Yaw = GetYawCharacterFromWall(HitDataMiddel);
	CheckDeltaVector = CharLoc - ClimbingChar->GetActorLocation();

	return true;
}

bool FClimbingPawnModeClimb::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	float Angle = VectorXYAngle(ClimbingChar->GetControlRotation().Vector(), ClimbingChar->GetActorForwardVector());


	if (fabs(Angle) > 90)
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
		MovementComponent.Velocity = ClimbingChar->GetControlRotation().Vector() * MovementComponent.ClimbJumpVelocyty;

		ReturnValue = true;
		return false;
	}
	else
	{
		if (MovementComponent.GetMode(EClimbingPawnModeType::LiftOnWall).CanSetMode())
		{
			MovementComponent.SetClimbMode(EClimbingPawnModeType::LiftOnWall);
		}
	}
	ReturnValue = false;
	return false;
}

//***********************************************
//FClimbingPawnModeLeftWallRun
//***********************************************

bool FClimbingPawnModeLeftWallRun::Tick(float DeltaTime)
{
	FVector NewPosition(0,0,0);
	FRotator NewRotation(0,0,0);
	FVector DesiredMovementThisFrame = MovementComponent.ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * MovementComponent.ClimbVelocyty;

	FVector OldVelocity = MovementComponent.Velocity;
	const FVector Gravity(0, 0, MovementComponent.GetGravityZ());

	MovementComponent.Velocity = MovementComponent.NewFallVelocity(MovementComponent.Velocity, Gravity, DeltaTime);

	FHitResult Hit(1);
	
	FVector Adjusted;

	

	if (MovementComponent.Velocity.Z > 0)
	{
		Adjusted = 0.5*(OldVelocity + MovementComponent.Velocity) * DeltaTime;
	}
	else
	{
		Adjusted = 0.5*(OldVelocity + MovementComponent.Velocity) * DeltaTime + MovementComponent.WallRunInputVelocyty * DesiredMovementThisFrame;
	}
	
	if (!CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
	{

		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
		
	}
	else
	{
		MovementComponent.MoveTo(NewPosition, NewRotation);
	}
	

	return false;
}

void FClimbingPawnModeLeftWallRun::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	
	FVector StartPosition;
	FRotator StartRotation;

	CheckDeltaVectorInCurrentStateSimple(StartPosition, StartRotation);

	MovementComponent.MoveTo(StartPosition, StartRotation);
		
	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;

}

void FClimbingPawnModeLeftWallRun::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	ClimbingChar->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;

	BlockState(0.5);

}

bool FClimbingPawnModeLeftWallRun::CanSetMode()
{
	if (IsStateBlock()) return false;

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());


	//Left Back Ray
	FVector LeftBackStart = ClimbingChar->GetActorLocation() + FVector(0, 0, 90);
	FVector LeftBackEnd = LeftBackStart + ClimbingChar->GetActorRightVector() * 60 * GetRayEndSign();
	//Left Forward Ray
	FVector LeftForwardStart = ClimbingChar->GetActorLocation() + FVector(0, 0, 90) + ClimbingChar->GetActorForwardVector() * 30;
	FVector LeftForwardEnd = LeftForwardStart + ClimbingChar->GetActorRightVector() * 60 * GetRayEndSign();


	FHitResult LeftBackHit;

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, LeftBackStart, LeftBackEnd, LeftBackHit) &&
		TraceLine(MovementComponent.GetWorld(), ClimbingChar, LeftForwardStart, LeftForwardEnd) &&
		ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0)
	{
		return true;
	}
	else return false;
}

bool FClimbingPawnModeLeftWallRun::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	if (ChackNeedStop()) return false;
	
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	
	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);
	//Left Back Ray
	FVector LeftBackStart = ClimbingChar->GetActorLocation() + InputDeltaVector;
	FVector LeftBackEnd = LeftBackStart + ClimbingChar->GetActorRightVector() * 60 * GetRayEndSign();

	//Left Forward Ray
	FVector LeftForwardStart = ClimbingChar->GetActorLocation() + InputDeltaVector + ClimbingChar->GetActorForwardVector() * 30;
	FVector LeftForwardEnd = LeftForwardStart + ClimbingChar->GetActorRightVector() * 60 * GetRayEndSign();

	FHitResult LeftBackHit;
	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, LeftBackStart, LeftBackEnd, LeftBackHit) &&
		TraceLine(MovementComponent.GetWorld(), ClimbingChar, LeftForwardStart, LeftForwardEnd))
	{
		CheckDeltaVector = LeftBackHit.Location + LeftBackHit.Normal * MovementComponent.WallOffset - ClimbingChar->GetActorLocation();
		CheckRotation.Yaw = LeftBackHit.Normal.Rotation().Yaw + GetCharRotation();
		return true;
	}
	
	else return false;
	
}

bool FClimbingPawnModeLeftWallRun::ChackNeedStop()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FVector Start = ClimbingChar->GetActorLocation();
	FVector End = Start - FVector(0, 0, 100);

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
	{
		return true;
	}

	Start = ClimbingChar->GetActorLocation();
	End = Start + ClimbingChar->GetActorForwardVector() * 50;

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
	{
		return true;
	}

	return false;
}

bool FClimbingPawnModeLeftWallRun::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());


	MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
			
	MovementComponent.Velocity = ClimbingChar->GetControlRotation().Vector()  * MovementComponent.WallRunJumpForwardVelocyty + FVector(0,0,1) * MovementComponent.WallRunJumpUpVelocyty;

	ReturnValue = true;
	return false;
	
}

//***********************************************
//FClimbingPawnModeRightWallRun
//***********************************************



//***********************************************
//FClimbingPawnModeSlide
//***********************************************

bool FClimbingPawnModeSlide::Tick(float DeltaTime)
{
	if (CheckDeltaVectorInCurrentStateSimple())
	{
		MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.SlideVelocytyCurve.GetRichCurve()->Eval(MinSlideTime);

		FVector Adjusted = MovementComponent.Velocity * DeltaTime;
		MovementComponent.MoveTo(Adjusted, MovementComponent.GetPawnOwner()->GetActorRotation());
		MinSlideTime += DeltaTime;
		
	}
	else
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	}
	

	return true;
}

void FClimbingPawnModeSlide::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.SlideVelocytyCurve.GetRichCurve()->GetTimeRange(MinSlideTime, MaxSlideTime);
	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;
	ClimbingChar->Crouch();

}

void FClimbingPawnModeSlide::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;
}

bool FClimbingPawnModeSlide::CanSetMode()
{

	float XYVelocyty = MovementComponent.Velocity.Size2D();
	float MaxVelocyty;
	float MinVelocyty;

	MovementComponent.RunVelocytyCurve.GetRichCurve()->GetValueRange(MinVelocyty, MaxVelocyty);
	float ThresholdVel = MovementComponent.SlideThreshold * (MaxVelocyty - MinVelocyty) + MinVelocyty;
	if (!MovementComponent.IsFalling() && XYVelocyty > ThresholdVel)
	{
		return true;
	}
	else return false;
}

bool FClimbingPawnModeSlide::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	if (MinSlideTime >= MaxSlideTime) return false;
	

	FVector Start = ClimbingChar->GetActorLocation() - ClimbingChar->GetActorForwardVector() * 60;
	FVector End = Start - FVector(0, 0, 100);

	if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
	{
		return false;
	}

	Start = ClimbingChar->GetActorLocation() - FVector(0, 0, 30);
	FVector EndLeft = Start + ClimbingChar->GetActorForwardVector() * 80 - ClimbingChar->GetActorRightVector() * 40;
	FVector EndRight = Start + ClimbingChar->GetActorForwardVector() * 80 + ClimbingChar->GetActorRightVector() * 40;

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, EndLeft) ||
		TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, EndRight))
	{
		return false;
	}

	else return true;
}

//***********************************************
//FClimbingPawnModeJumpOverBarier
//***********************************************

bool FClimbingPawnModeJumpOverBarier::Tick(float DeltaTime)
{
	FVector NewAdjusted;
	FRotator NewRotation;
	FVector Adjusted;

	MovementComponent.Velocity.Z = 0;

	if (MovementComponent.Velocity.Size() > MovementComponent.JumpOverBarierMinLiftVelocyty)
	{
		Adjusted = MovementComponent.Velocity * DeltaTime;
		Adjusted.Z = MovementComponent.Velocity.Size() * DeltaTime;
	}
	else
	{
		Adjusted = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.JumpOverBarierMinLiftVelocyty * DeltaTime;
		Adjusted.Z = MovementComponent.JumpOverBarierMinLiftVelocyty * DeltaTime;
	}

	if (CheckDeltaVectorInCurrentState(Adjusted, NewAdjusted, NewRotation))
	{
		MovementComponent.MoveTo(NewAdjusted, NewRotation);
		
		
	}
	else
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	}
	

	return false;
}

void FClimbingPawnModeJumpOverBarier::UnSetMode()
{
	BlockState(0.5);
	
	if (MovementComponent.Velocity.Size() < MovementComponent.JumpOverBarierMinLiftVelocyty)
	{
		MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.JumpOverBarierMinLiftVelocyty;
	}
}

bool FClimbingPawnModeJumpOverBarier::CanSetMode()
{
	if (IsStateBlock()) return false;

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FVector Start = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, MovementComponent.JumpOverBarierDeltaJumpHeght);
	FVector End = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, -90);

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
	{
		return true;
	}
	else return false;
}

bool FClimbingPawnModeJumpOverBarier::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	CheckRotation = ClimbingChar->GetActorRotation();

	FHitResult HitResultForward;
	FVector Start = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, MovementComponent.JumpOverBarierDeltaJumpHeght);
	FVector End = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, -120);

	TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitResultForward);

	if (ClimbingChar->GetActorLocation().Z - HitResultForward.ImpactPoint.Z < 100)
	{
		CheckDeltaVector = FVector(0, 0, InputDeltaVector.Z);
		return true;
	}

	else return false;
}

//***********************************************
//FClimbingPawnModeUnderWallJump
//***********************************************

bool FClimbingPawnModeUnderWallJump::CanSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	//Left Ray
	FVector LeftStart = ClimbingChar->GetActorLocation() + FVector(0, 0, 90) - ClimbingChar->GetActorRightVector() * 40;
	FVector LeftEnd = LeftStart + ClimbingChar->GetActorForwardVector() * 70;

	//Right Ray
	FVector RightStart = ClimbingChar->GetActorLocation() + FVector(0, 0, 90) + ClimbingChar->GetActorRightVector() * 40;
	FVector RightEnd = RightStart + ClimbingChar->GetActorForwardVector() * 70;

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, RightStart, RightEnd) &&
		TraceLine(MovementComponent.GetWorld(), ClimbingChar, LeftStart, LeftEnd) &&
		ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0)
	{
		return true;
	}
	else return false;
}

//***********************************************
//FClimbingPawnModeZipLine
//***********************************************

bool FClimbingPawnModeZipLine::Tick(float DeltaTime)
{
	FVector NewPosition;
	FRotator NewRotation;

	MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.ZipLineVelocyty;
	FVector Adjusted = MovementComponent.Velocity * DeltaTime;

	if (CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
	{
		MovementComponent.MoveTo(NewPosition, NewRotation);
		
	}
	else
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	}
	

	return false;
}

void FClimbingPawnModeZipLine::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	FVector StartPosition;
	FRotator StartRotation;

	CheckDeltaVectorInCurrentStateSimple(StartPosition, StartRotation);

	MovementComponent.MoveTo(StartPosition, StartRotation);

	if (ClimbingChar->bFistPirsonView) MovementComponent.CameraRotate(ClimbingChar->GetActorRotation());
	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;



}

void FClimbingPawnModeZipLine::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;
}

bool FClimbingPawnModeZipLine::CanSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	ClimbingChar->ZipLine = Cast<AZipLine>(ClimbingChar->OverlopObject);
	if (!ClimbingChar->ZipLine->EndBox->IsOverlappingActor(ClimbingChar))
	{
		return true;

	}
	else return false;
}

bool FClimbingPawnModeZipLine::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	if (ClimbingChar->ZipLine->EndBox->IsOverlappingActor(ClimbingChar))
	{
		return false;
	}

	CheckDeltaVector = ClimbingChar->ZipLine->Spline->FindLocationClosestToWorldLocation(
		ClimbingChar->GetActorLocation() - FVector(0, 0, MovementComponent.ZipLineDeltaZ) + InputDeltaVector, ESplineCoordinateSpace::World) + FVector(0, 0, MovementComponent.ZipLineDeltaZ) - ClimbingChar->GetActorLocation();
	CheckRotation.Yaw = ClimbingChar->ZipLine->Spline->FindRotationClosestToWorldLocation(
		ClimbingChar->GetActorLocation() - FVector(0, 0, MovementComponent.ZipLineDeltaZ) + InputDeltaVector, ESplineCoordinateSpace::World).Yaw;

	return true;
}

bool FClimbingPawnModeZipLine::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	MovementComponent.Velocity = ClimbingChar->GetControlRotation().Vector()  * MovementComponent.ZipLineJumpVelocyty;

	ReturnValue = true;
	return false;
}

//***********************************************
//FClimbingPawnModeInclinedSlide
//***********************************************

bool FClimbingPawnModeInclinedSlide::Tick(float DeltaTime)
{
	FVector NewPosition;
	FRotator NewRotation;
	FVector DesiredMovementThisFrame = MovementComponent.ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * MovementComponent.ClimbVelocyty;

	const FVector Gravity(0.f, 0.f, MovementComponent.GetGravityZ());

	MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.InclinedSlideVelosytyForward;
	MovementComponent.Velocity += MovementComponent.NewFallVelocity(MovementComponent.Velocity, Gravity, DeltaTime);


	FVector Adjusted = MovementComponent.Velocity * DeltaTime + DesiredMovementThisFrame;

	if (CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
	{
		MovementComponent.MoveTo(NewPosition, NewRotation);
	}
	else
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	}
	

	return false;
}

void FClimbingPawnModeInclinedSlide::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	FVector StartPosition;
	FRotator StartRotation;

	CheckDeltaVectorInCurrentStateSimple(StartPosition, StartRotation);

	MovementComponent.MoveTo(StartPosition, StartRotation);

	if (ClimbingChar->bFistPirsonView) MovementComponent.CameraRotate(ClimbingChar->GetActorRotation());
	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;

}

void FClimbingPawnModeInclinedSlide::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;

	FRotator NewRot = FRotator(0, ClimbingChar->GetActorRotation().Yaw, 0);
	MovementComponent.MoveTo(FVector(0), NewRot);

	BlockState(0.5);
}

bool FClimbingPawnModeInclinedSlide::CanSetMode()
{
	if (MovementComponent.IsFalling() || IsStateBlock()) return false;

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FVector Start = ClimbingChar->GetActorLocation();
	FVector End = Start - FVector(0, 0, 130);


	FHitResult HitResult;
	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitResult) && 90 - HitResult.Normal.Rotation().Pitch > MovementComponent.InclinedSlideAngle)
	{
		return true;
	}
	else
	{
		BlockState(0.2);
		return false;
	}
	
}

bool FClimbingPawnModeInclinedSlide::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	if (MovementComponent.IsFalling()) return false;
	FVector Start = ClimbingChar->GetActorLocation() + InputDeltaVector;
	FVector End = Start - FVector(0, 0, 130);
	FHitResult HitResult;

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitResult) && 90 - HitResult.Normal.Rotation().Pitch > MovementComponent.InclinedSlideAngle)
	{
		CheckRotation = HitResult.Normal.Rotation();
		CheckRotation.Pitch -= 90;
		CheckDeltaVector = InputDeltaVector;

		return true;
	}

	else return false;
	
}

bool FClimbingPawnModeInclinedSlide::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	MovementComponent.SetMovementMode(MOVE_Falling);
	MovementComponent.Velocity = ClimbingChar->GetControlRotation().Vector()  * MovementComponent.ZipLineJumpVelocyty;
	MovementComponent.Velocity += FVector(0, 0, MovementComponent.JumpZVelocity);

	ReturnValue = true;
	return false;
}

//***********************************************
//FClimbingPawnModeLiftOnWall
//***********************************************

bool FClimbingPawnModeLiftOnWall::Tick(float DeltaTime)
{
	FVector NewPosition;
	FRotator NewRotation;
	FVector Adjusted;

	Adjusted = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.ClimbLiftVelocyty * DeltaTime;
	Adjusted.Z = MovementComponent.ClimbLiftVelocyty * DeltaTime;

	if (CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
	{
		MovementComponent.MoveTo(NewPosition, NewRotation);
	}
	else
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	}
	
	

	return false;
}

void FClimbingPawnModeLiftOnWall::UnSetMode()
{
	if (MovementComponent.Velocity.Size() < MovementComponent.JumpOverBarierMinLiftVelocyty)
	{
		MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.ClimbLiftVelocyty;
	}
}

bool FClimbingPawnModeLiftOnWall::CanSetMode()
{

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FVector Start = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, 100);
	FVector End = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, -120);

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
	{
		return true;
	}
	else return false;
}

bool FClimbingPawnModeLiftOnWall::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	CheckRotation = ClimbingChar->GetActorRotation();

	FHitResult HitResultForward;
	FVector Start = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, 100);
	FVector End = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, -120);

	TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End, HitResultForward);


	if (ClimbingChar->GetActorLocation().Z - HitResultForward.ImpactPoint.Z < 100)
	{
		CheckDeltaVector = FVector(0, 0, InputDeltaVector.Z);
		return true;
	}

	else return false;
}

//***********************************************
//FClimbingPawnModeRoundingTheCorner
//***********************************************

bool FClimbingPawnModeRoundingTheCorner::Tick(float DeltaTime)
{
	FVector NewPosition;
	FRotator NewRotation;
	FVector Adjusted;

	Adjusted.X = MovementComponent.RoundingTheCornerVelocyty * DeltaTime;
	Adjusted.Y = MovementComponent.RoundingTheCornerRotationVelocyty * DeltaTime;
	

	if (CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
	{
		MovementComponent.MoveTo(NewPosition, NewRotation);
	}
	else
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Climb);
		
	}
	
	

	return false;
}

void FClimbingPawnModeRoundingTheCorner::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;

	if (ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveRight")) > 0)
	{
		RoundingTheCornerData.MovementDirection = ERoundingCornerDirection::Right;
	}
	else
	{
		RoundingTheCornerData.MovementDirection = ERoundingCornerDirection::Left;
	}

	FHitResult TopHitResult;
	FVector TopStart = ClimbingChar->GetActorLocation() + FVector(0, 0, 97) + ClimbingChar->GetActorForwardVector() * 60;
	FVector TopEnd = ClimbingChar->GetActorLocation() + FVector(0, 0, -97) + ClimbingChar->GetActorForwardVector() * 60;
	TraceLine(MovementComponent.GetWorld(), ClimbingChar, TopStart, TopEnd, TopHitResult);

	RoundingTheCornerData.State = ERoundingCornerState::FistMove;
	RoundingTheCornerData.TraceLineZ = TopHitResult.ImpactPoint.Z - 10 - ClimbingChar->GetActorLocation().Z;
	RoundingTheCornerData.RotateAngle = 0;

}

void FClimbingPawnModeRoundingTheCorner::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;
}

bool FClimbingPawnModeRoundingTheCorner::CanSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FHitResult TopHitResult;
	FVector TopStart = ClimbingChar->GetActorLocation() + FVector(0, 0, 97) + ClimbingChar->GetActorForwardVector() * 60;
	FVector TopEnd = TopStart - FVector(0, 0, 200);

	if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, TopStart, TopEnd, TopHitResult)) return false;

	FVector UnderRayStart = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorRightVector() * 50 + ClimbingChar->GetActorForwardVector() * 100;
	UnderRayStart.Z = TopHitResult.ImpactPoint.Z - 10;
	FVector UnderRayEnd = UnderRayStart - ClimbingChar->GetActorRightVector() * 100;

	FVector OverRayStart = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorRightVector() * 50 + ClimbingChar->GetActorForwardVector() * 100;
	OverRayStart.Z = TopHitResult.ImpactPoint.Z + 10;
	FVector OverRayEnd = OverRayStart - ClimbingChar->GetActorRightVector() * 100;

	if (ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveRight")) > 0 && TraceLine(MovementComponent.GetWorld(), ClimbingChar, UnderRayStart, UnderRayEnd) &&
		!TraceLine(MovementComponent.GetWorld(), ClimbingChar, OverRayStart, OverRayEnd))
	{
		return true;
	}
	else if (ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveRight")) < 0 && TraceLine(MovementComponent.GetWorld(), ClimbingChar, UnderRayEnd, UnderRayStart) &&
		!TraceLine(MovementComponent.GetWorld(), ClimbingChar, OverRayEnd, OverRayStart))
	{
		return true;
	}

	else return false;
}

bool FClimbingPawnModeRoundingTheCorner::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	FVector Start;
	FVector End;

	switch (RoundingTheCornerData.State)
	
	{
	case ERoundingCornerState::FistMove:

		CheckRotation = ClimbingChar->GetActorRotation();
		if (RoundingTheCornerData.MovementDirection == ERoundingCornerDirection::Right)
		{
			Start = ClimbingChar->GetActorLocation() + FVector(0, 0, RoundingTheCornerData.TraceLineZ) - ClimbingChar->GetActorRightVector() * 40;
			End = Start + ClimbingChar->GetActorForwardVector() * 60;

			if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
			{
				CheckDeltaVector = ClimbingChar->GetActorRightVector() * InputDeltaVector.X;
			}
			else
			{
				RoundingTheCornerData.State = ERoundingCornerState::Rotate;
			}
		}
		else
		{
			Start = ClimbingChar->GetActorLocation() + FVector(0, 0, RoundingTheCornerData.TraceLineZ) + ClimbingChar->GetActorRightVector() * 40;
			End = Start + ClimbingChar->GetActorForwardVector() * 60;

			if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
			{
				CheckDeltaVector = ClimbingChar->GetActorRightVector() * (-InputDeltaVector.X);
			}
			else
			{
				RoundingTheCornerData.State = ERoundingCornerState::Rotate;
			}
			
		}

		return true;
	
	case ERoundingCornerState::Rotate:
	
		RoundingTheCornerData.RotateAngle = RoundingTheCornerData.RotateAngle + InputDeltaVector.Y;

		if (RoundingTheCornerData.MovementDirection == ERoundingCornerDirection::Right)
		{
			if (RoundingTheCornerData.RotateAngle > 90)
			{
				CheckRotation.Yaw = ClimbingChar->GetActorRotation().Yaw - InputDeltaVector.Y - (RoundingTheCornerData.RotateAngle - 90);
				RoundingTheCornerData.State = ERoundingCornerState::SecondMove;
			}
			else
			{
				CheckRotation.Yaw = ClimbingChar->GetActorRotation().Yaw - InputDeltaVector.Y;
			}
		}
		else
		{
			if (RoundingTheCornerData.RotateAngle > 90)
			{
				CheckRotation.Yaw = ClimbingChar->GetActorRotation().Yaw + InputDeltaVector.Y - (RoundingTheCornerData.RotateAngle - 90);
				RoundingTheCornerData.State = ERoundingCornerState::SecondMove;
			}
			else
			{
				CheckRotation.Yaw = ClimbingChar->GetActorRotation().Yaw + InputDeltaVector.Y;
			}

		}
		return true;
	
	case ERoundingCornerState::SecondMove:
	
		CheckRotation = ClimbingChar->GetActorRotation();
		if (RoundingTheCornerData.MovementDirection == ERoundingCornerDirection::Right)
		{
			Start = ClimbingChar->GetActorLocation() + FVector(0, 0, RoundingTheCornerData.TraceLineZ) - ClimbingChar->GetActorRightVector() * 45;
			End = Start + ClimbingChar->GetActorForwardVector() * 60;

			if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
			{
				CheckDeltaVector = ClimbingChar->GetActorRightVector() * InputDeltaVector.X;

				return true;
			}

			else return false;
			
		}
		else
		{
			Start = ClimbingChar->GetActorLocation() + FVector(0, 0, RoundingTheCornerData.TraceLineZ) + ClimbingChar->GetActorRightVector() * 45;
			End = Start + ClimbingChar->GetActorForwardVector() * 60;

			if (!TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End))
			{
				CheckDeltaVector = ClimbingChar->GetActorRightVector() * (-InputDeltaVector.X);

				return true;
			}
			else return false;
		}

	default: throw FString("Error");
	}

}

	