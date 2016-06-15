// Copyright 2016 Dmitriy

#include "ClimbingSystem.h"
#include "ClimbingPawnMovementComponent.h"
#include "ClimbingCharacter.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "OverlapObject.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"


bool Lib::TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, EDrawDebugTrace::Type DrawDebugType, ECollisionChannel CollisionChannel, bool ReturnPhysMat)
{
	FHitResult HitOut;
	return TraceLine(World, ActorToIgnore, Start, End, HitOut, DrawDebugType, CollisionChannel, ReturnPhysMat);
}
bool Lib::TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, FHitResult& HitOut, EDrawDebugTrace::Type DrawDebugType, ECollisionChannel CollisionChannel, bool ReturnPhysMat)
{
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams TraceParams(FName(TEXT("Param Trace")), true, ActorToIgnore);
	TraceParams.bTraceComplex = true;
	//TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;

	//Ignore Actors
	TraceParams.AddIgnoredActor(ActorToIgnore);

	//Re-initialize hit info
	HitOut = FHitResult(ForceInit);


	//Trace!
	World->LineTraceSingleByChannel(
		HitOut,		//result
		Start,	//start
		End, //end
		CollisionChannel, //collision channel
		TraceParams
		);

	if (DrawDebugType == EDrawDebugTrace::ForOneFrame)
	{
		DrawDebugLine(World, Start, HitOut.bBlockingHit ? HitOut.Location : End, FColor::Red, false, -1.f, SDPG_World, 2.f);
		if (HitOut.bBlockingHit)
		{
			DrawDebugPoint(World, HitOut.Location, 20.f, FColor::Red, false, -1.f, SDPG_World);
			DrawDebugLine(World, HitOut.Location, End, FColor::Green, false, -1.f, SDPG_World, 2.f);
		}
	}
	else if (DrawDebugType == EDrawDebugTrace::ForDuration)
	{
		float DrawTime = 2.f;
		DrawDebugLine(World, Start, HitOut.bBlockingHit ? HitOut.Location : End, FColor::Red, false, DrawTime, SDPG_World, 2.f);
		if (HitOut.bBlockingHit)
		{
			DrawDebugPoint(World, HitOut.Location, 20.f, FColor::Red, false, DrawTime, SDPG_World);
			DrawDebugLine(World, HitOut.Location, End, FColor::Green, false, DrawTime, SDPG_World, 2.f);
		}
	}
	else if (DrawDebugType == EDrawDebugTrace::Persistent)
	{
		DrawDebugLine(World, Start, HitOut.bBlockingHit ? HitOut.Location : End, FColor::Red, true, -1.f, SDPG_World, 2.f);
		if (HitOut.bBlockingHit)
		{
			DrawDebugPoint(World, HitOut.Location, 20.f, FColor::Red, true, -1.f, SDPG_World);
			DrawDebugLine(World, HitOut.Location, End, FColor::Green, true, -1.f, SDPG_World, 2.f);
		}
	}
	//Hit any Actor?
	return HitOut.bBlockingHit;
}

float Lib::VectorXYAngle(FVector V1, FVector V2)
{
	float V1Len = pow(pow(V1.X, 2) + pow(V1.Y, 2), 0.5);
	float V2Len = pow(pow(V2.X, 2) + pow(V2.Y, 2), 0.5);

	float ScalarProduct = (V1.X * V2.X + V1.Y * V2.Y);
	float CosVek = ScalarProduct / V1Len * V2Len;
	float B = V1.X * V2.Y - V2.X * V1.Y;
	float Sign = B > 0 ? 1 : -1;
	return acos(CosVek)  * 180.0 / PI * Sign;
}

void Lib::Msg(FString Message, bool OnScreen, bool OnLog)
{
	if (OnScreen) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, Message);
	if (OnLog) UE_LOG(ClimbingSystem, Warning, TEXT("%s"), *Message);

}
void Lib::Msg(FVector Vector, bool OnScreen, bool OnLog)
{
	Msg(Vector.ToString(), OnScreen, OnLog);
}

void Lib::Msg(float Message, bool OnScreen, bool OnLog)
{
	Msg(FString::SanitizeFloat(Message), OnScreen, OnLog);
}

UClimbingPawnMovementComponent::UClimbingPawnMovementComponent(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
	ClimbDeltaZ = -50;
	ClimbVelocyty = 150;
	ClimbJumpVelocyty = 600;

	MaxWalkSpeed = 500;
	RunSpeed = 800;
	RunDelay = 2;

	UnderWallJumpZVelocyty = 700;
	JumpZVelocity = 400;
	NavAgentProps.bCanCrouch = true;
	bOrientRotationToMovement = true;

	WallRunJumpZVelocyty = 600;
	WallOffset = 50;
	WallRunLimitAngle = 0;
	WallRunFallGravityScale = 0.2;
	WallRunInputVelocyty = 3;
	WallRunJumpVelocyty = 600;

	ZipLineDeltaZ = -90;
	ZipLineVelocyty = 800;
	ZipLineJumpVelocyty = 600;

	SlideVelocytyCurve.GetRichCurve()->AddKey(0, RunSpeed);
	SlideVelocytyCurve.GetRichCurve()->AddKey(0.8, 0.625 * RunSpeed);
	SlideVelocytyCurve.GetRichCurve()->AddKey(1.2, 0);

	BlockInclinedSlide = true;
	InclinedSlideAngle = 40;
	InclinedSlideVelosytyForward = 400;
	InclinedSlideVelosytyRight = 200;
	InclinedJumpVelocyty = 600;
}

// Called when the game starts or when spawned
void UClimbingPawnMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
		
}

void  UClimbingPawnMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{

	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(PawnOwner);
	
	// Make sure that everything is still valid, and that we are allowed to move.
	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	if (ClimbingMode == EClimbingMode::CLIMB_None)
	{
		DefineClimbMode(); //Define Climb mode
	}

	switch (ClimbingMode)
	{
	case EClimbingMode::CLIMB_Climb:
	{
		FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * ClimbVelocyty;
		FVector NewPosition;
		FRotator NewRotation;

		if (!DesiredMovementThisFrame.IsNearlyZero())
		{
			if (CheckDeltaVectorInCurrentState(DesiredMovementThisFrame, NewPosition, NewRotation))
			{
				MoveTo(NewPosition, NewRotation);
			}
		}
		break;
	}
	case EClimbingMode::CLIMB_LeftWallRun:
	case EClimbingMode::CLIMB_RightWallRun:
	{
		FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * ClimbVelocyty;
		FVector NewPosition;
		FRotator NewRotation;

		if (Velocity.Z > 0)
		{
			FVector OldVelocity = Velocity;
			const FVector Gravity(0.f, 0.f, GetGravityZ());
			Velocity = NewFallVelocity(Velocity, Gravity, DeltaTime);
			FHitResult Hit(1.f);
			FVector Adjusted = 0.5f*(OldVelocity + Velocity) * DeltaTime;
			if (!CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
			{
				SetClimbMode(EClimbingMode::CLIMB_None);
				return;
			}
			MoveTo(NewPosition, NewRotation);
		}

		else
		{
			FVector OldVelocity = Velocity;
			const FVector Gravity(0.f, 0.f, GetGravityZ() * WallRunFallGravityScale);
			Velocity = NewFallVelocity(Velocity, Gravity, DeltaTime);
			FHitResult Hit(1.f);
			FVector Adjusted = 0.5f*(OldVelocity + Velocity) * DeltaTime + WallRunInputVelocyty * DesiredMovementThisFrame;
			if (!CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
			{
				SetClimbMode(EClimbingMode::CLIMB_None);
				return;
			}
			MoveTo(NewPosition, NewRotation);
		}
		break;
	}

	case EClimbingMode::CLIMB_JumpOverBarier:
	{
		if (!CheckDeltaVectorInCurrentState())
		{
			SetClimbMode(EClimbingMode::CLIMB_None);
			return;
		}

		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		break;
	}

	

	case EClimbingMode::CLIMB_Slide:
	{
		if (!CheckDeltaVectorInCurrentState())
		{
			SetClimbMode(EClimbingMode::CLIMB_None);
			return;
		}

		Velocity = PawnOwner->GetActorForwardVector() * SlideVelocytyCurve.GetRichCurve()->Eval(MinSlideTime);
		
		FVector Adjusted = Velocity * DeltaTime;
		MoveTo(Adjusted, PawnOwner->GetActorRotation());
		MinSlideTime += DeltaTime;

		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		break;
	}

	case EClimbingMode::CLIMB_ZipLine:
	{
		FVector NewPosition;
		FRotator NewRotation;
		Velocity = PawnOwner->GetActorForwardVector() * ZipLineVelocyty;
		FVector Adjusted = Velocity * DeltaTime;
		
		if (!CheckDeltaVectorInCurrentState(Adjusted,NewPosition, NewRotation))
		{
			SetClimbMode(EClimbingMode::CLIMB_None);
			return;
		}
		
		MoveTo(NewPosition, NewRotation);

		break;
	}

	case EClimbingMode::CLIMB_InclinedSlide:
	{
		FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f) * DeltaTime * InclinedSlideVelosytyRight;
		FVector NewPosition;
		FRotator NewRotation;
		const FVector Gravity(0.f, 0.f, GetGravityZ());

		Velocity = PawnOwner->GetActorForwardVector() * InclinedSlideVelosytyForward;
		Velocity += NewFallVelocity(Velocity, Gravity, DeltaTime);
		 

		FVector Adjusted = Velocity * DeltaTime + DesiredMovementThisFrame;

		if (!CheckDeltaVectorInCurrentState(Adjusted, NewPosition, NewRotation))
		{
			SetClimbMode(EClimbingMode::CLIMB_None);
			return;
		}
		
		MoveTo(NewPosition, NewRotation);

	

		break;
	}

	default:
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
		break;
	}
};

void UClimbingPawnMovementComponent::DefineClimbMode()
{
	
	if (CanSetClimbMode(EClimbingMode::CLIMB_Climb))
	{
		SetClimbMode(EClimbingMode::CLIMB_Climb);
	}
	else if (IsFalling() && CanSetClimbMode(EClimbingMode::CLIMB_LeftWallRun))
	{
		SetClimbMode(EClimbingMode::CLIMB_LeftWallRun);
	}
	else if (IsFalling() && CanSetClimbMode(EClimbingMode::CLIMB_RightWallRun))
	{
		SetClimbMode(EClimbingMode::CLIMB_RightWallRun);
	}
	else if (CanSetClimbMode(EClimbingMode::CLIMB_InclinedSlide))
	{
		SetClimbMode(EClimbingMode::CLIMB_InclinedSlide);
	}

	if (!bIsRun)
	{
		if (PawnOwner->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 && !Cast<AClimbingCharacter>(PawnOwner)->bIsCrouched && !IsFalling())
		{
			if (!GetWorld()->GetTimerManager().IsTimerActive(RunTimerHandle))
			{
				GetWorld()->GetTimerManager().SetTimer(RunTimerHandle, this, &UClimbingPawnMovementComponent::SetRun, RunDelay, false);
			}

		}
		else
		{
			if (GetWorld()->GetTimerManager().IsTimerActive(RunTimerHandle))
			{
				GetWorld()->GetTimerManager().ClearTimer(RunTimerHandle);
			}
		}
	}
	else
	{
		float XYVelocyty = pow(pow(Velocity.X, 2) + pow(Velocity.Y, 2), 0.5);

		if (XYVelocyty < 400)
		{
			bIsRun = false;
		}
	}

	



	
}

void UClimbingPawnMovementComponent::SetRun()
{
	bIsRun = true;
}

void UClimbingPawnMovementComponent::SetClimbMode(EClimbingMode _ClimbingMode)
{
	
	if (ClimbingMode == _ClimbingMode) return;
	

	LastClimbingMode = ClimbingMode;
	ClimbingMode = _ClimbingMode;

	UnSetMode(LastClimbingMode);
	if (!SetMode(ClimbingMode))
	{
		SetMode(LastClimbingMode);
		ClimbingMode = LastClimbingMode;
		UE_LOG(ClimbingSystem, Warning, TEXT("Cannot change mode"));
		return;
	}
	
}



EClimbingMode UClimbingPawnMovementComponent::GetClimbingMode() const
{
	return ClimbingMode;
}

bool UClimbingPawnMovementComponent::SetMode(EClimbingMode ClimbingMode)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(PawnOwner);
	FVector StartPosition;
	FRotator StartRotation;

	switch (ClimbingMode)
	{
	case EClimbingMode::CLIMB_Climb:
	{
		
		if (!CheckDeltaVectorInCurrentState(StartPosition, StartRotation)) return false;
		MoveTo(StartPosition, StartRotation);

		PawnOwner->bUseControllerRotationYaw = false;
		bOrientRotationToMovement = false;
		Velocity = FVector(0, 0, 0);
		break;
	}
	case EClimbingMode::CLIMB_JumpOverBarier:

		Cast<AClimbingCharacter>(PawnOwner)->ClimbCapsule->SetCapsuleHalfHeight(30);
		break;

	case EClimbingMode::CLIMB_LeftWallRun:
	case EClimbingMode::CLIMB_RightWallRun:
	{
		if (!CheckDeltaVectorInCurrentState(StartPosition, StartRotation)) return false;

		MoveTo(StartPosition, StartRotation);
		PawnOwner->bUseControllerRotationYaw = false;
		bOrientRotationToMovement = false;

		break;
	}
	case EClimbingMode::CLIMB_Slide:
	{
		SlideVelocytyCurve.GetRichCurve()->GetTimeRange(MinSlideTime, MaxSlideTime);
		PawnOwner->bUseControllerRotationYaw = false;
		bOrientRotationToMovement = false;
		ClimbingChar->Crouch();
		
		break;

	}
	case EClimbingMode::CLIMB_ZipLine:
	{
		if (!CheckDeltaVectorInCurrentState(StartPosition, StartRotation)) return false;

		MoveTo(StartPosition, StartRotation);
		PawnOwner->bUseControllerRotationYaw = false;
		bOrientRotationToMovement = false;

		break;
	}
	case EClimbingMode::CLIMB_InclinedSlide:
	{
		if (!CheckDeltaVectorInCurrentState(StartPosition, StartRotation)) return false;
		MoveTo(StartPosition, StartRotation);
		PawnOwner->bUseControllerRotationYaw = false;
		bOrientRotationToMovement = false;
		
		break;
	
	}
	}

	return true;
}
void UClimbingPawnMovementComponent::UnSetMode(EClimbingMode ClimbingMode)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(PawnOwner);

	switch (ClimbingMode)
	{
	case EClimbingMode::CLIMB_Climb:
	{
		BlockClimb = true;
		PawnOwner->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
		bOrientRotationToMovement = true;
		FTimerHandle BlockTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(BlockTimerHandle, this, &UClimbingPawnMovementComponent::UnblockClimbState, 0.5, false);
		break;
	}

	case EClimbingMode::CLIMB_LeftWallRun:
	case EClimbingMode::CLIMB_RightWallRun:
	{
		PawnOwner->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
		bOrientRotationToMovement = true;
		BlockWallRun = true;
		FTimerHandle BlockTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(BlockTimerHandle, this, &UClimbingPawnMovementComponent::UnblockWallRunState, 0.5, false);
		break;
	}
	case EClimbingMode::CLIMB_JumpOverBarier:

		Cast<AClimbingCharacter>(PawnOwner)->ClimbCapsule->SetCapsuleHalfHeight(96);
		break;

	case EClimbingMode::CLIMB_Slide:
	{
		PawnOwner->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
		bOrientRotationToMovement = true;
		break;
	}
	case EClimbingMode::CLIMB_ZipLine:
	{
		PawnOwner->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
		bOrientRotationToMovement = true;
		
		break;
	}
	case EClimbingMode::CLIMB_InclinedSlide:
	{
		PawnOwner->bUseControllerRotationYaw = ClimbingChar->bFistPirsonView;
		bOrientRotationToMovement = true;
		FRotator NewRot = FRotator(0, ClimbingChar->GetActorRotation().Yaw, 0);
		MoveTo(FVector(0), NewRot);
		break;
	}

	}
}

void UClimbingPawnMovementComponent::UnblockClimbState()
{
	BlockClimb = false;
}

void UClimbingPawnMovementComponent::UnblockWallRunState()
{
	BlockWallRun = false;
}

bool UClimbingPawnMovementComponent::CanSetClimbMode(EClimbingMode ClimbingMode)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(PawnOwner);

	switch (ClimbingMode)
	{

	case EClimbingMode::CLIMB_Climb:
	{
		if (BlockClimb) return false;
		FVector Start = PawnOwner->GetActorLocation() + FVector(0, 0, 97) + PawnOwner->GetActorForwardVector() * 60;
		FVector End = Start - FVector(0, 0, 97);

		if (IsFalling() && Lib::TraceLine(GetWorld(), PawnOwner, Start, End))
		{
			return true;
		}
		break;
	}

	case EClimbingMode::CLIMB_UnderWallJump:
	{
		//Left Ray
		FVector LeftStart = PawnOwner->GetActorLocation() + FVector(0, 0, 90) - PawnOwner->GetActorRightVector() * 40;
		FVector LeftEnd = LeftStart + PawnOwner->GetActorForwardVector() * 70;
		//Right Ray
		FVector RightStart = PawnOwner->GetActorLocation() + FVector(0, 0, 90) + PawnOwner->GetActorRightVector() * 40;
		FVector RightEnd = RightStart + PawnOwner->GetActorForwardVector() * 70;

		if (Lib::TraceLine(GetWorld(), PawnOwner, RightStart, RightEnd) &&
			Lib::TraceLine(GetWorld(), PawnOwner, LeftStart, LeftEnd) &&
			PawnOwner->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0)
		{
			return true;
		}
		break;
	}
	case EClimbingMode::CLIMB_JumpOverBarier:
	{
		//Bottom Ray
		FVector BottomStart = PawnOwner->GetActorLocation();
		FVector BottomEnd = BottomStart + PawnOwner->GetActorForwardVector() * 300;
		//Top Ray
		FVector TopStart = BottomStart + FVector(0, 0, 90);
		FVector TopEnd = TopStart + PawnOwner->GetActorForwardVector() * 300;

		if (Lib::TraceLine(GetWorld(), PawnOwner, BottomStart, BottomEnd) &&
			!Lib::TraceLine(GetWorld(), PawnOwner, TopStart, TopEnd))
		{
			return true;
		}
		break;
	}

	case EClimbingMode::CLIMB_LeftWallRun:
	{
		if (BlockWallRun) return false;
		//Left Back Ray
		FVector LeftBackStart = PawnOwner->GetActorLocation() + FVector(0, 0, 90);
		FVector LeftBackEnd = LeftBackStart - PawnOwner->GetActorRightVector() * 60;
		//Left Forward Ray
		FVector LeftForwardStart = PawnOwner->GetActorLocation() + FVector(0, 0, 90) + PawnOwner->GetActorForwardVector() * 30;
		FVector LeftForwardEnd = LeftForwardStart - PawnOwner->GetActorRightVector() * 60;


		FHitResult LeftBackHit;

		if (Lib::TraceLine(GetWorld(), PawnOwner, LeftBackStart, LeftBackEnd, LeftBackHit) &&
			Lib::TraceLine(GetWorld(), PawnOwner, LeftForwardStart, LeftForwardEnd) &&
			PawnOwner->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 &&
			Lib::VectorXYAngle(PawnOwner->GetActorForwardVector(), LeftBackHit.Normal) > WallRunLimitAngle + 90)
		{
			return true;
		}
		break;
	}
	case EClimbingMode::CLIMB_RightWallRun:
	{
		if (BlockWallRun) return false;
		//Right Back Ray
		FVector RightBackStart = PawnOwner->GetActorLocation() + FVector(0, 0, 90);
		FVector RightBackEnd = RightBackStart + PawnOwner->GetActorRightVector() * 60;
		//Right Forward Ray
		FVector RightForwardStart = PawnOwner->GetActorLocation() + FVector(0, 0, 90) + PawnOwner->GetActorForwardVector() * 30;
		FVector RightForwardEnd = RightForwardStart + PawnOwner->GetActorRightVector() * 60;

		FHitResult RightBackHit;

		if (Lib::TraceLine(GetWorld(), PawnOwner, RightBackStart, RightBackEnd, RightBackHit) &&
			Lib::TraceLine(GetWorld(), PawnOwner, RightForwardStart, RightForwardEnd) &&
			PawnOwner->InputComponent->GetAxisValue(TEXT("MoveForward")) > 0 &&
			Lib::VectorXYAngle(PawnOwner->GetActorForwardVector(), RightBackHit.Normal) > WallRunLimitAngle + 90)
		{
			return true;
		}
		break;
	}
	case EClimbingMode::CLIMB_Slide:
	{
		if (!IsFalling() && bIsRun)
		{
			return true;
		}
		break;
	}
	case EClimbingMode::CLIMB_ZipLine:

		ClimbingChar->ZipLine = Cast<AZipLine>(ClimbingChar->OverlopObject);
		if (!ClimbingChar->ZipLine->EndBox->IsOverlappingActor(ClimbingChar))
		{
			return true;

		}
		break;

	case EClimbingMode::CLIMB_InclinedSlide:
	{
		if (IsFalling()) return false;
		FVector Start = PawnOwner->GetActorLocation();
		FVector End = Start - FVector(0, 0, 130);
		
		
		FHitResult HitResult;
		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End, HitResult) && 90 - HitResult.Normal.Rotation().Pitch > InclinedSlideAngle)
		{
				if (BlockInclinedSlide)
				{
					if (!GetWorld()->GetTimerManager().IsTimerActive(InclinedSlideTimerHandle))
					{
						GetWorld()->GetTimerManager().SetTimer(InclinedSlideTimerHandle, this, &UClimbingPawnMovementComponent::UnBlockInclinedSlide, 0.2, false);
					}
					return false;
				}
				else
				{
					
					BlockInclinedSlide = true;
					return true;
				}
					
		}
		else
		{
			BlockInclinedSlide = true;
			return false;
		}
		
		
		break;
	}
	}

	return false;
}
void UClimbingPawnMovementComponent::UnBlockInclinedSlide()
{
	BlockInclinedSlide = false;
}

bool UClimbingPawnMovementComponent::CheckDeltaVectorInCurrentState(FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	return CheckDeltaVectorInCurrentState(FVector(0, 0, 0), CheckDeltaVector, CheckRotation);
}
bool UClimbingPawnMovementComponent::CheckDeltaVectorInCurrentState()
{
	FVector CheckDeltaVector;
	FRotator CheckRotation;

	return CheckDeltaVectorInCurrentState(FVector(0, 0, 0), CheckDeltaVector, CheckRotation);
}

bool UClimbingPawnMovementComponent::CheckDeltaVectorInCurrentState(const FVector& InputDeltaVector, FVector& CheckDeltaVector, FRotator& CheckRotation)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(PawnOwner);
	CheckDeltaVector = FVector(0, 0, 0);
	CheckRotation = FRotator(0, 0, 0);

	switch (ClimbingMode)
	{
	case EClimbingMode::CLIMB_Climb:
	{
		int DeltaReyHeight = -5;
		//Top Ray
		FVector Start = PawnOwner->GetActorLocation() + InputDeltaVector + FVector(0, 0, 97) + PawnOwner->GetActorForwardVector() * 60;
		FVector End = Start - FVector(0, 0, 97);
		FHitResult HitDataTop;
		FHitResult HitDataMiddel;
		if (!Lib::TraceLine(GetWorld(), PawnOwner, Start, End, HitDataTop)) return false;
		for (int i = 4; i >= 0; i--)
		{

			Start = FVector((GetActorLocation() + InputDeltaVector).X, (GetActorLocation() + InputDeltaVector).Y, HitDataTop.Location.Z - (10 * pow(2, i)));
			End = Start + PawnOwner->GetActorForwardVector() * 60;

			if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End, HitDataMiddel))
			{
				DeltaReyHeight = i;
				break;
			}
		}

		if (DeltaReyHeight == -5) return false;

		//Left Ray
		Start = FVector((GetActorLocation() + InputDeltaVector).X, (GetActorLocation() + InputDeltaVector).Y, HitDataTop.Location.Z - (10 * pow(2, DeltaReyHeight))) - PawnOwner->GetActorRightVector() * 40;
		End = Start + PawnOwner->GetActorForwardVector() * 70;
		FHitResult HitDataLeft;
		//UE_LOG(ClimbingSystem, Warning, TEXT("1"));
		if (!Lib::TraceLine(GetWorld(), PawnOwner, Start, End, HitDataLeft)) return false;

		//Right Ray
		Start = FVector((GetActorLocation() + InputDeltaVector).X, (GetActorLocation() + InputDeltaVector).Y, HitDataTop.Location.Z - (10 * pow(2, DeltaReyHeight))) + PawnOwner->GetActorRightVector() * 40;
		End = Start + PawnOwner->GetActorForwardVector() * 70;
		FHitResult HitDataRight;
		if (!Lib::TraceLine(GetWorld(), PawnOwner, Start, End, HitDataRight)) return false;

		FVector CharLoc = HitDataMiddel.Location;
		CharLoc.Z = HitDataTop.Location.Z + ClimbDeltaZ;
		float CharYaw;
		if (HitDataLeft.Normal.Rotation().Yaw < 0 && HitDataRight.Normal.Rotation().Yaw > 0)
		{
			CharYaw = HitDataMiddel.Normal.Rotation().Yaw;
		}
		else
		{
			CharYaw = HitDataMiddel.Normal.Rotation().Yaw + 180;
		}

		CheckRotation.Yaw = CharYaw;
		CheckDeltaVector = CharLoc - GetActorLocation();
		return true;
	}

	case EClimbingMode::CLIMB_LeftWallRun:
	{
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0, 0, 100);
		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End))
		{
			return false;
		}
		Start = PawnOwner->GetActorLocation();
		End = Start + PawnOwner->GetActorForwardVector() * 50;
		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End))
		{
			return false;
		}

		//Left Back Ray
		FVector LeftBackStart = GetActorLocation() + InputDeltaVector;
		FVector LeftBackEnd = LeftBackStart - PawnOwner->GetActorRightVector() * 60;
		//Left Forward Ray
		FVector LeftForwardStart = GetActorLocation() + InputDeltaVector + PawnOwner->GetActorForwardVector() * 30;
		FVector LeftForwardEnd = LeftForwardStart - PawnOwner->GetActorRightVector() * 60;

		FHitResult LeftBackHit;
		if (Lib::TraceLine(GetWorld(), PawnOwner, LeftBackStart, LeftBackEnd, LeftBackHit) &&
			Lib::TraceLine(GetWorld(), PawnOwner, LeftForwardStart, LeftForwardEnd))
		{
			CheckDeltaVector = LeftBackHit.Location + LeftBackHit.Normal * WallOffset - GetActorLocation();
			CheckRotation.Yaw = LeftBackHit.Normal.Rotation().Yaw + 270;
			return true;
		}

		return false;

	}

	case EClimbingMode::CLIMB_RightWallRun:
	{

		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0, 0, 100);
		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End))
		{
			return false;
		}
		Start = PawnOwner->GetActorLocation();
		End = Start + PawnOwner->GetActorForwardVector() * 50;
		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End))
		{
			return false;
		}

		//Right Back Ray
		FVector RightBackStart = GetActorLocation() + InputDeltaVector;
		FVector RightBackEnd = RightBackStart + PawnOwner->GetActorRightVector() * 60;
		//Right Forward Ray
		FVector RightForwardStart = GetActorLocation() + InputDeltaVector + PawnOwner->GetActorForwardVector() * 30;
		FVector RightForwardEnd = RightForwardStart + PawnOwner->GetActorRightVector() * 60;

		FHitResult RightBackHit;

		if (Lib::TraceLine(GetWorld(), PawnOwner, RightBackStart, RightBackEnd, RightBackHit) &&
			Lib::TraceLine(GetWorld(), PawnOwner, RightForwardStart, RightForwardEnd))
		{
			CheckDeltaVector = RightBackHit.Location + RightBackHit.Normal * WallOffset - GetActorLocation();
			CheckRotation.Yaw = RightBackHit.Normal.Rotation().Yaw + 90;
			
			return true;
		}

		return false;
	}

	case EClimbingMode::CLIMB_Slide:
	{
		if (MinSlideTime >= MaxSlideTime)
		{
			return false;
		}

		FVector Start = GetActorLocation() - PawnOwner->GetActorForwardVector() * 60;
		FVector End = Start - FVector(0, 0, 100);

		if (!Lib::TraceLine(GetWorld(), PawnOwner, Start, End))
		{
			return false;
		}

		Start = PawnOwner->GetActorLocation() - FVector(0, 0, 30);
		FVector EndLeft = Start + PawnOwner->GetActorForwardVector() * 80 - PawnOwner->GetActorRightVector() * 40;
		FVector EndRight = Start + PawnOwner->GetActorForwardVector() * 80 + PawnOwner->GetActorRightVector() * 40;

		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, EndLeft) ||
			Lib::TraceLine(GetWorld(), PawnOwner, Start, EndRight))
		{
			return false;
		}

		return true;
		
	}

	case EClimbingMode::CLIMB_JumpOverBarier:
	{
		FVector Start = GetActorLocation();
		FVector End = Start - FVector(0, 0, 100);
		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End))
		{
			return false;
		}

		return true;
	}
	case EClimbingMode::CLIMB_ZipLine:
	{
		if (ClimbingChar->ZipLine->EndBox->IsOverlappingActor(ClimbingChar))
		{
			return false;
		}

		CheckDeltaVector = ClimbingChar->ZipLine->Spline->FindLocationClosestToWorldLocation(GetActorLocation() - FVector(0, 0, ZipLineDeltaZ) + InputDeltaVector, ESplineCoordinateSpace::World) + FVector(0,0, ZipLineDeltaZ) - GetActorLocation();
		CheckRotation.Yaw = ClimbingChar->ZipLine->Spline ->FindRotationClosestToWorldLocation(GetActorLocation() - FVector(0, 0, ZipLineDeltaZ) + InputDeltaVector, ESplineCoordinateSpace::World).Yaw;

		return true;
	}
	case EClimbingMode::CLIMB_InclinedSlide:
	{
		if (IsFalling()) return false;
		FVector Start = ClimbingChar->GetActorLocation() + InputDeltaVector;
		FVector End = Start - FVector(0, 0, 130);
		FHitResult HitResult;
		if (Lib::TraceLine(GetWorld(), PawnOwner, Start, End, HitResult) && 90 - HitResult.Normal.Rotation().Pitch > InclinedSlideAngle)
		{
			CheckRotation = HitResult.Normal.Rotation();
			CheckRotation.Pitch -= 90;
			CheckDeltaVector = InputDeltaVector;
			
			
			return true;
		}
		else
		{
			return false;
		}
	}
	}
	return false;
}


bool UClimbingPawnMovementComponent::DoJump(bool bReplayingMoves)
{
	AClimbingCharacter* ClimbingChar = Cast<AClimbingCharacter>(PawnOwner);

	switch (ClimbingMode)
	{
	case EClimbingMode::CLIMB_None:

		if (CharacterOwner && CharacterOwner->CanJump())
		{
			// Don't jump if we can't move up/down.
			if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
			{

				if (CanSetClimbMode(EClimbingMode::CLIMB_UnderWallJump))
				{
					Velocity = FVector(0, 0, UnderWallJumpZVelocyty);
					SetMovementMode(MOVE_Falling);
				}
				else if (CanSetClimbMode(EClimbingMode::CLIMB_JumpOverBarier))
				{
					SetClimbMode(EClimbingMode::CLIMB_JumpOverBarier);
					PawnOwner->SetActorLocation(PawnOwner->GetActorLocation() + FVector(0, 0, 120));
					Velocity += PawnOwner->GetActorForwardVector() * 250;
					SetMovementMode(MOVE_Falling);

				}
				else if (CanSetClimbMode(EClimbingMode::CLIMB_LeftWallRun))
				{
					SetClimbMode(EClimbingMode::CLIMB_LeftWallRun);
					Velocity.Z = WallRunJumpZVelocyty;
					SetMovementMode(MOVE_Falling);

				}
				else if (CanSetClimbMode(EClimbingMode::CLIMB_RightWallRun))
				{
					SetClimbMode(EClimbingMode::CLIMB_RightWallRun);
					Velocity.Z = WallRunJumpZVelocyty;
					SetMovementMode(MOVE_Falling);

				}
				else
				{
					Velocity.Z = JumpZVelocity;
					SetMovementMode(MOVE_Falling);

				}
				return true;
			}
		}

		break;

	case EClimbingMode::CLIMB_Climb:
	{
		float Angle = Lib::VectorXYAngle(UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation()), ClimbingChar->GetActorForwardVector());
		if (fabs(Angle) > 90)
		{
			SetClimbMode(EClimbingMode::CLIMB_None);
			Velocity = UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation())  * ClimbJumpVelocyty;
		}
		else
		{
			SetClimbMode(EClimbingMode::CLIMB_None);
			Velocity = FVector(0, 0, 600);
		}
		break;
	}

	case EClimbingMode::CLIMB_LeftWallRun:
	{
		float Angle;
		//Left Back Ray
		FVector LeftBackStart = GetActorLocation();
		FVector LeftBackEnd = LeftBackStart - PawnOwner->GetActorRightVector() * 60;

		FHitResult LeftBackHit;
		if (Lib::TraceLine(GetWorld(), PawnOwner, LeftBackStart, LeftBackEnd, LeftBackHit))
		{
			Angle = Lib::VectorXYAngle(UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation()),LeftBackHit.Normal);
			if (fabs(Angle) < 90)
			{
				SetClimbMode(EClimbingMode::CLIMB_None);
				Velocity = UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation())  * WallRunJumpVelocyty;
			}
		}
	}

	case EClimbingMode::CLIMB_RightWallRun:
	{
		float Angle;
		//Right Back Ray
		FVector RightBackStart = GetActorLocation();
		FVector RightBackEnd = RightBackStart + PawnOwner->GetActorRightVector() * 60;

		FHitResult RightBackHit;
		if (Lib::TraceLine(GetWorld(), PawnOwner, RightBackStart, RightBackEnd, RightBackHit))
		{
			Angle = Lib::VectorXYAngle(UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation()), RightBackHit.Normal);
			if (fabs(Angle) < 90)
			{
				SetClimbMode(EClimbingMode::CLIMB_None);
				Velocity = UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation())  * WallRunJumpVelocyty;
			}
		}
	}

	case EClimbingMode::CLIMB_ZipLine:
	{
		
			
		SetClimbMode(EClimbingMode::CLIMB_None);
		Velocity = UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation())  * ZipLineJumpVelocyty;
			
		
	}
	case EClimbingMode::CLIMB_InclinedSlide:
	{
		SetClimbMode(EClimbingMode::CLIMB_None);
		SetMovementMode(MOVE_Falling);
		Velocity = UKismetMathLibrary::GetForwardVector(ClimbingChar->GetControlRotation())  * ZipLineJumpVelocyty;
		Velocity += FVector(0,0,JumpZVelocity);
	}
	}
	

	return false;
}


float UClimbingPawnMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();
	
	
	if (bIsRun)
	{
		MaxSpeed = RunSpeed;
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

