// Copyright 2016 - 2018 Dmitriy Pavlov
#include "ClimbingPawnMode.h"
#include "ClimbingPawnMovementComponent.h"
#include "ClimbingCharacter.h"
#include "TimerManager.h"
#include "OverlapObject.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "Components/InputComponent.h"

//***********************************************
//EClimbingPawnModeType
//***********************************************

static_assert(EClimbingPawnModeType::end == (EClimbingPawnModeType)12, "Fix EClimbingPawnModeTypeCreate");

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

	case EClimbingPawnModeType::LadderMove:

		return new FClimbingPawnModeLadderMove(MovementComponent);

	default: throw;

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

void FClimbingPawnModeBase::BlockState(std::chrono::milliseconds BlockTime)
{
	FTimerHandle BlockTimerHandle;

	bBlockState = true;
	UnblockStateFuture = std::async(std::launch::async, UnblockState, this, BlockTime);
}

void FClimbingPawnModeBase::UnblockState(FClimbingPawnModeBase* ClimbingPawnMode, std::chrono::milliseconds BlockTime)
{
	std::this_thread::sleep_for(BlockTime);

	ClimbingPawnMode->bBlockState = false;
}

//***********************************************
//FClimbingPawnModeRun
//***********************************************

FClimbingPawnModeRun::FClimbingPawnModeRun(UClimbingPawnMovementComponent& MovementComponent) : FClimbingPawnModeBase(MovementComponent)
{
	
}

bool FClimbingPawnModeRun::Tick(float DeltaTime)
{
	DefineRunSpeed(DeltaTime);
	DefineClimbMode();
	FallingControl();
	
	return true;
}

void FClimbingPawnModeRun::FallingControl()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	FRotator ControlRot = ClimbingChar->GetControlRotation();

	FVector StartRightTop = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorRightVector() * 40 + ClimbingChar->GetActorUpVector() * 90;
	FVector EndRightTop = StartRightTop + ClimbingChar->GetActorForwardVector() * 48;

	FVector StartLeftTop = ClimbingChar->GetActorLocation() - ClimbingChar->GetActorRightVector() * 40 + ClimbingChar->GetActorUpVector() * 90;
	FVector EndLeftTop = StartLeftTop + ClimbingChar->GetActorForwardVector() * 48;

	FVector StartRightBottom = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorRightVector() * 40 - ClimbingChar->GetActorUpVector() * 90;
	FVector EndRightBottom = StartRightBottom + ClimbingChar->GetActorForwardVector() * 48;

	FVector StartLeftBottom = ClimbingChar->GetActorLocation() - ClimbingChar->GetActorRightVector() * 40 - ClimbingChar->GetActorUpVector() * 90;
	FVector EndLeftBottom = StartLeftBottom + ClimbingChar->GetActorForwardVector() * 48;


	if (MovementComponent.IsFalling() && MovementComponent.Velocity.Z < 0 && ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 && ControlRot.Pitch > 270 && 
		ControlRot.Pitch < 320 &&
		!TraceLine(MovementComponent.GetWorld(), ClimbingChar, StartRightTop, EndRightTop) && 
		!TraceLine(MovementComponent.GetWorld(), ClimbingChar, StartLeftTop, EndLeftTop) && 
		!TraceLine(MovementComponent.GetWorld(), ClimbingChar, StartRightBottom, EndRightBottom) && 
		!TraceLine(MovementComponent.GetWorld(), ClimbingChar, StartLeftBottom, EndLeftBottom))
	{
		MovementComponent.Velocity = MovementComponent.Velocity.Size() * ClimbingChar->GetControlRotation().Vector();
	}
}

bool FClimbingPawnModeRun::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	if (MovementComponent.GetMode(EClimbingPawnModeType::LeftWallRun).CanSetMode())
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::LeftWallRun);
		MovementComponent.SetMovementMode(MOVE_Falling);

	}
	else if (MovementComponent.GetMode(EClimbingPawnModeType::RightWallRun).CanSetMode())
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::RightWallRun);
		MovementComponent.SetMovementMode(MOVE_Falling);

	}
	else if (MovementComponent.GetMode(EClimbingPawnModeType::UnderWallJump).CanSetMode())
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::UnderWallJump);
		MovementComponent.SetMovementMode(MOVE_Falling);
	}

	else return true;
	

	ReturnValue = true;

	return false;

}


void FClimbingPawnModeRun::DefineClimbMode()
{

	if (MovementComponent.IsFalling())
	{
		if (MovementComponent.GetMode(EClimbingPawnModeType::Climb).CanSetMode()) MovementComponent.SetClimbMode(EClimbingPawnModeType::Climb);

		if (MovementComponent.GetMode(EClimbingPawnModeType::JumpOverBarier).CanSetMode()) MovementComponent.SetClimbMode(EClimbingPawnModeType::JumpOverBarier);

		if (MovementComponent.GetMode(EClimbingPawnModeType::LeftWallRun).CanSetMode()) MovementComponent.SetClimbMode(EClimbingPawnModeType::LeftWallRun);

		if (MovementComponent.GetMode(EClimbingPawnModeType::RightWallRun).CanSetMode()) MovementComponent.SetClimbMode(EClimbingPawnModeType::RightWallRun);
	}
	else
	{
		if (MovementComponent.GetMode(EClimbingPawnModeType::InclinedSlide).CanSetMode()) MovementComponent.SetClimbMode(EClimbingPawnModeType::InclinedSlide);
	}

}

void FClimbingPawnModeRun::DefineRunSpeed(float DeltaTime)
{
	
	if (MovementComponent.IsFalling())
	{
		return;
	}


	else if (MovementComponent.ClimbingChar->bIsCrouched || MovementComponent.Velocity.Size() < MovementComponent.MinRunVelocyty - 100)
	{
		MovementComponent.CurrentRunTime = 0;
	}

	if (MovementComponent.CurrentRunTime < MovementComponent.MaxRunTime && MovementComponent.ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0)
	{
		MovementComponent.CurrentRunTime += DeltaTime;

	}

}


//***********************************************
//FClimbingPawnModeClimb
//***********************************************

bool FClimbingPawnModeClimb::Tick(float DeltaTime)
{
	FVector NewPosition;
	FRotator NewRotation;

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FVector DesiredMovementThisFrame = MovementComponent.ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * MovementComponent.ClimbVelocyty;

	

	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		if (CheckDeltaVectorInCurrentState(DesiredMovementThisFrame, NewPosition, NewRotation))
		{
			ClimbingChar->InputDeltaVector = NewPosition;
			MovementComponent.MoveTo(NewPosition, NewRotation, true);
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

	MovementComponent.MoveTo(StartPosition, StartRotation, true);

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	ClimbingChar->BlockCameraYawRangeFromCharacter(150, 150);

	MovementComponent.CurrentRunTime = 0;
}

void FClimbingPawnModeClimb::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	
	BlockState(std::chrono::milliseconds(500));

	ClimbingChar->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;
	ClimbingChar->UnBlockCameraYawRange();

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
	else if (ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveRight")) > 0)
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

	FVector CharLoc = HitDataMiddel.Location + HitDataMiddel.Normal * MovementComponent.ClimbDeltaWallOffset;
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
		MovementComponent.MoveTo(NewPosition, NewRotation, true);
	}
	

	return false;
}

void FClimbingPawnModeLeftWallRun::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	

	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;

	MovementComponent.Velocity.Z = MovementComponent.WallRunJumpOnWallZVelocyty;
	if (ClimbingChar->bFistPirsonView)
	{
		MovementComponent.YawCameraSet(ClimbingChar->GetActorRotation().Yaw);

		MovementComponent.RollCameraSet(-MovementComponent.WallRunCameraRoll * GetRayEndSign());

		if (GetType() == EClimbingPawnModeType::LeftWallRun)
		{
			ClimbingChar->BlockCameraYawRangeFromCharacter(0, 120);
		}
		else if (GetType() == EClimbingPawnModeType::RightWallRun)
		{
			ClimbingChar->BlockCameraYawRangeFromCharacter(120, 0);
		}
	}
	
}

void FClimbingPawnModeLeftWallRun::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	ClimbingChar->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;

	MovementComponent.bOrientRotationToMovement = true;

	MovementComponent.RollCameraSet(0);
		
	ClimbingChar->UnBlockCameraYawRange();
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
		ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 && ClimbingChar->bPressedJump)
	{
		return true;
	}
	return false;
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

	FVector JumpVelocety;
	
	float Angle = VectorXYAngle(ClimbingChar->GetControlRotation().Vector(), ClimbingChar->GetActorForwardVector());

	FVector StartUpRay = ClimbingChar->GetActorLocation();
	FVector EndUpRay = StartUpRay + ClimbingChar->GetActorUpVector() * 200;

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, StartUpRay, EndUpRay))
	{
		JumpVelocety = ClimbingChar->GetControlRotation().Vector() * MovementComponent.WallRunJumpForwardVelocyty * 2;
	}
	else if ((Angle * GetRayEndSign()) > -50  && (Angle  * GetRayEndSign()) < 15)
	{
		JumpVelocety = ClimbingChar->GetActorForwardVector() * MovementComponent.WallRunJumpForwardVelocyty + ClimbingChar->GetActorRightVector() * 50 * -GetRayEndSign() +
			FVector(0, 0, 1) * MovementComponent.WallRunJumpUpVelocyty;
	}
	else
	{
		JumpVelocety = ClimbingChar->GetControlRotation().Vector()  * MovementComponent.WallRunJumpForwardVelocyty + FVector(0, 0, 1) * MovementComponent.WallRunJumpUpVelocyty;
		
	}


	MovementComponent.Velocity = JumpVelocety;

	ReturnValue = true;
	return false;
	
}





//***********************************************
//FClimbingPawnModeSlide
//***********************************************

bool FClimbingPawnModeSlide::Tick(float DeltaTime)
{
	if (CheckDeltaVectorInCurrentStateSimple())
	{
		MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.SlideVelocytyCurve.GetRichCurve()->Eval(MinSlideTime);

		FVector Adjusted = MovementComponent.Velocity * DeltaTime;
		MovementComponent.MoveTo(Adjusted, MovementComponent.GetPawnOwner()->GetActorRotation(), true);
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
	if (ClimbingChar->bFistPirsonView) ClimbingChar->BlockCameraYawRangeFromCharacter(90, 90);
}

void FClimbingPawnModeSlide::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;
	ClimbingChar->UnBlockCameraYawRange();
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
		MovementComponent.MoveTo(NewAdjusted, NewRotation, true);
		
	}
	else
	{
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	}
	

	return false;
}

void FClimbingPawnModeJumpOverBarier::UnSetMode()
{
	BlockState(std::chrono::milliseconds(200));
	
	if (MovementComponent.Velocity.Size() < MovementComponent.JumpOverBarierMinLiftVelocyty)
	{
		MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.JumpOverBarierMinLiftVelocyty;
	}
	else
	{
		MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.Velocity.Size();
	}
}

bool FClimbingPawnModeJumpOverBarier::CanSetMode()
{
	if (IsStateBlock()) return false;

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FVector Start = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, MovementComponent.JumpOverBarierDeltaJumpHeght);
	FVector End = ClimbingChar->GetActorLocation() + ClimbingChar->GetActorForwardVector() * 60 + FVector(0, 0, -90);

	if (ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 && TraceLine(MovementComponent.GetWorld(), ClimbingChar, Start, End) &&
		!TraceLine(MovementComponent.GetWorld(), ClimbingChar, ClimbingChar->GetActorLocation() + FVector(0, 0, MovementComponent.JumpOverBarierDeltaJumpHeght), Start))
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
	FVector LeftEnd = LeftStart + ClimbingChar->GetActorForwardVector() * MovementComponent.WallJumpDistance;

	//Right Ray
	FVector RightStart = ClimbingChar->GetActorLocation() + FVector(0, 0, 90) + ClimbingChar->GetActorRightVector() * 40;
	FVector RightEnd = RightStart + ClimbingChar->GetActorForwardVector() * MovementComponent.WallJumpDistance;

	if (TraceLine(MovementComponent.GetWorld(), ClimbingChar, RightStart, RightEnd) &&
		TraceLine(MovementComponent.GetWorld(), ClimbingChar, LeftStart, LeftEnd) &&
		ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0)
	{
		return true;
	}
	else return false;
}

void FClimbingPawnModeUnderWallJump::SetMode()
{

	CharRotashen = MovementComponent.GetPawnOwner()->GetActorRotation().Yaw;

	MovementComponent.Velocity = FVector(0, 0, MovementComponent.UnderWallJumpZVelocyty) + MovementComponent.GetPawnOwner()->GetActorForwardVector() * 500;

}

bool FClimbingPawnModeUnderWallJump::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	float Angle = std::abs(CharRotashen - MovementComponent.GetPawnOwner()->GetActorRotation().Yaw);
	if (Angle > 140 && Angle < 220)
	{
		MovementComponent.Velocity = MovementComponent.GetPawnOwner()->GetActorForwardVector() * MovementComponent.UnderWallJumpFromWallForwardVelocyty +
			FVector(0,0, MovementComponent.UnderWallJumpFromWallZVelocyty);
		MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	}
	return true;
}

bool FClimbingPawnModeUnderWallJump::Tick(float DeltaTime)
{
	if (!MovementComponent.IsFalling())  MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);

	if (MovementComponent.GetMode(EClimbingPawnModeType::Climb).CanSetMode()) MovementComponent.SetClimbMode(EClimbingPawnModeType::Climb);

	return true;
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
		MovementComponent.MoveTo(NewPosition, NewRotation, false);
		
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

	MovementComponent.MoveTo(StartPosition, StartRotation, false);

	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;

	if (ClimbingChar->bFistPirsonView) MovementComponent.YawCameraSet(ClimbingChar->GetActorRotation().Yaw);

	if (ClimbingChar->bFistPirsonView) ClimbingChar->BlockCameraYawRangeFromCharacter(90, 90);

}

void FClimbingPawnModeZipLine::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;

	ClimbingChar->UnBlockCameraYawRange();
}

bool FClimbingPawnModeZipLine::CanSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CurrentZipLine = Cast<AZipLine>(ClimbingChar->OverlopObject);
	if (CurrentZipLine && !CurrentZipLine->EndBox->IsOverlappingActor(ClimbingChar))
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

	if (CurrentZipLine->EndBox->IsOverlappingActor(ClimbingChar))
	{
		return false;
	}

	CheckDeltaVector = CurrentZipLine->Spline->FindLocationClosestToWorldLocation(
		ClimbingChar->GetActorLocation() - FVector(0, 0, MovementComponent.ZipLineDeltaZ) + InputDeltaVector, ESplineCoordinateSpace::World) + FVector(0, 0, MovementComponent.ZipLineDeltaZ) - ClimbingChar->GetActorLocation();
	CheckRotation.Yaw = CurrentZipLine->Spline->FindRotationClosestToWorldLocation(
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
		MovementComponent.MoveTo(NewPosition, NewRotation, true);
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

	MovementComponent.MoveTo(StartPosition, StartRotation,true);

	if (ClimbingChar->bFistPirsonView) MovementComponent.YawCameraSet(ClimbingChar->GetActorRotation().Yaw);
	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;
	if (ClimbingChar->bFistPirsonView) ClimbingChar->BlockCameraYawRangeFromCharacter(90, 90);
}

void FClimbingPawnModeInclinedSlide::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;

	FRotator NewRot = FRotator(0, ClimbingChar->GetActorRotation().Yaw, 0);
	
	ClimbingChar->SetActorRotation(NewRot);
	BlockState(std::chrono::milliseconds(200));

	ClimbingChar->UnBlockCameraYawRange();
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
		BlockState(std::chrono::milliseconds(200));
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

void FClimbingPawnModeLiftOnWall::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;
	ClimbingChar->BlockCameraYawRangeFromCharacter(90, 90);
	MovementComponent.SetMovementMode(MOVE_Flying);
	MovementComponent.GravityScale = 0;
}

void FClimbingPawnModeLiftOnWall::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	
	ClimbingChar->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;
	ClimbingChar->UnBlockCameraYawRange();

	
	MovementComponent.SetMovementMode(MOVE_Walking);
	MovementComponent.GravityScale = 1;
	
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
		MovementComponent.MoveTo(NewPosition, NewRotation, false);
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

	if (ClimbingChar->bFistPirsonView) ClimbingChar->BlockCameraYawRangeFromCharacter(150, 150);
}

void FClimbingPawnModeRoundingTheCorner::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;

	ClimbingChar->UnBlockCameraYawRange();
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
			if (ClimbingChar->bFistPirsonView) MovementComponent.AddYawCamera(-InputDeltaVector.Y);

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
			if (ClimbingChar->bFistPirsonView) MovementComponent.AddYawCamera(InputDeltaVector.Y);

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

	
//***********************************************
//FClimbingPawnModeLadderMove
//***********************************************

bool FClimbingPawnModeLadderMove::Tick(float DeltaTime)
{
	FVector NewPosition;
	FRotator NewRotation;
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());
	FVector DesiredMovementThisFrame = MovementComponent.ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * MovementComponent.LadderMoveVelocyty;

	if (!DesiredMovementThisFrame.IsNearlyZero())
	{
		if (CheckDeltaVectorInCurrentState(DesiredMovementThisFrame, NewPosition, NewRotation))
		{
			ClimbingChar->InputDeltaVector = NewPosition;
			MovementComponent.MoveTo(NewPosition, NewRotation, false);

		}
		else
		{
			MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
		}
		
	}
	
	
	return false;
}

void FClimbingPawnModeLadderMove::SetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	FVector StartPosition = CurrentLadder->GetActorLocation(); 
	FRotator StartRotation;

	DeltaRotate = CurrentLadder->LadderVolumeForward->IsOverlappingActor(ClimbingChar) ? 0 : 180;

	DeltaOffset = CurrentLadder->LadderVolumeForward->IsOverlappingActor(ClimbingChar) ?  - MovementComponent.DistnseToLadder - 10 : MovementComponent.DistnseToLadder;

	FVector AbsLocation = CurrentLadder->GetActorLocation();
	AbsLocation.Z = ClimbingChar->GetActorLocation().Z;

	if (AbsLocation.Z < CurrentLadder->GetActorLocation().Z + 50)
	{
		AbsLocation.Z = CurrentLadder->GetActorLocation().Z + 50;
		CurrentCharacterPosition = 0;
	}
	
	else
	{
		CurrentCharacterPosition = AbsLocation.Z - (CurrentLadder->GetActorLocation().Z + 50);
	}

	AbsLocation = AbsLocation + CurrentLadder->GetActorForwardVector() * DeltaOffset;


	ClimbingChar->SetActorLocationAndRotation(AbsLocation, FRotator(0, CurrentLadder->GetActorRotation().Yaw + DeltaRotate, 0));
	
	ClimbingChar->bUseControllerRotationYaw = false;
	MovementComponent.bOrientRotationToMovement = false;

	if (ClimbingChar->bFistPirsonView) MovementComponent.YawCameraSet(ClimbingChar->GetActorRotation().Yaw);

	if (ClimbingChar->bFistPirsonView) ClimbingChar->BlockCameraYawRangeFromCharacter(170, 170);

	BlockUnSet(std::chrono::milliseconds(600));
}

void FClimbingPawnModeLadderMove::UnSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.GetPawnOwner()->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
	MovementComponent.bOrientRotationToMovement = true;

	MovementComponent.Velocity = ClimbingChar->GetControlRotation().Vector()  * 250;

	ClimbingChar->UnBlockCameraYawRange();
}

bool FClimbingPawnModeLadderMove::CanSetMode()
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CurrentLadder = Cast<ALadder>(ClimbingChar->OverlopObject);

	if (CurrentLadder && ClimbingChar->GetActorLocation().Z > CurrentLadder->GetActorLocation().Z + 30 + CurrentLadder->LadderHeight) return false;

	else if (!CurrentLadder) return false;
	
	else return true;

	
	
}

bool FClimbingPawnModeLadderMove::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	FRotator ControlRot = ClimbingChar->GetControlRotation();


	if ((ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 && ControlRot.Pitch > 270 && ControlRot.Pitch < 360))
	{
		if (CurrentCharacterPosition < 50)
		{
			if (IsUnSetBlock())
			{
				CheckDeltaVector = FVector(0, 0, 0);
				CheckRotation.Yaw = CurrentLadder->GetActorRotation().Yaw + DeltaRotate;
				return true;
			}
			else
			{
				return false;
			}
			
		}
		CheckDeltaVector = FVector(0, 0, -InputDeltaVector.Size());
		CurrentCharacterPosition -= InputDeltaVector.Size();

	}
	else if (ClimbingChar->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 && ControlRot.Pitch > 0 && ControlRot.Pitch < 90)
	{
		if (CurrentCharacterPosition > CurrentLadder->LadderHeight - 170 && CurrentLadder->ClosedUp)
		{
			CheckDeltaVector = FVector(0, 0, 0);
			CheckRotation.Yaw = CurrentLadder->GetActorRotation().Yaw + DeltaRotate;
			return true;
		}
		else if (CurrentCharacterPosition > CurrentLadder->LadderHeight + 10)
		{
			
			return false;
			
		}
		CheckDeltaVector = FVector(0, 0, InputDeltaVector.Size());
		
		CurrentCharacterPosition += InputDeltaVector.Size();
	}

	CheckRotation.Yaw = CurrentLadder->GetActorRotation().Yaw + DeltaRotate;
	return true;
}

bool FClimbingPawnModeLadderMove::DoJump(bool bReplayingMoves, bool& ReturnValue)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(MovementComponent.GetPawnOwner());

	MovementComponent.SetClimbMode(EClimbingPawnModeType::Run);
	MovementComponent.Velocity = ClimbingChar->GetControlRotation().Vector()  * MovementComponent.ZipLineJumpVelocyty;

	ReturnValue = true;
	return false;
}


void FClimbingPawnModeLadderMove::BlockUnSet(std::chrono::milliseconds BlockTime)
{
	FTimerHandle BlockTimerHandle;

	bBlockUnset = true;
	UsetStateFuture = std::async(std::launch::async, UnBlockUnSet, this, BlockTime);
}

void FClimbingPawnModeLadderMove::UnBlockUnSet(FClimbingPawnModeLadderMove* ClimbingPawnMode, std::chrono::milliseconds BlockTime)
{
	std::this_thread::sleep_for(BlockTime);

	ClimbingPawnMode->bBlockUnset = false;
}