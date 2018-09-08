// Copyright 2016 - 2018 Dmitriy Pavlov
#include "ClimbingCharacter.h"
#include "ClimbingPawnMovementComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "OverlapObject.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ClimbingPawnMode.h"
 




// Sets default values
AClimbingCharacter::AClimbingCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UClimbingPawnMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	
	ClimbingMovement = Cast<UClimbingPawnMovementComponent>(Super::GetMovementComponent());

	TArray<USkeletalMeshComponent*> Components;
	GetComponents<USkeletalMeshComponent>(Components);
	USkeletalMeshComponent* ClimbMesh = Components[0];
	

	//Add Thrid pirson Camera arm
	ThridCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThridCameraSpringArm"));
	ThridCameraSpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	ThridCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 8.0f), FRotator(0.0f, 0.0f, 0.0f));
	ThridCameraSpringArm->TargetArmLength = 300.f;
	ThridCameraSpringArm->bUsePawnControlRotation = true;

	//Add Fist pirson Camera arm
	FistCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("FistCameraSpringArm"));
	FistCameraSpringArm->AttachToComponent(ClimbMesh, FAttachmentTransformRules::KeepRelativeTransform, FName("head"));
	FistCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 12.0f, 0.0f), FRotator(0.0f, 90.0f, 270.0f));
	FistCameraSpringArm->TargetArmLength = 0;
	FistCameraSpringArm->bUsePawnControlRotation = true;
	FistCameraSpringArm->bEnableCameraLag = true;

	FistCameraSpringArm->CameraLagSpeed = 65;
	FistCameraSpringArm->CameraLagMaxTimeStep = 0.5;

	if (ClimbingMovement->GetCurrentClimbingMode() != EClimbingPawnModeType::Climb)
	{
		bUseControllerRotationYaw = true;
		ClimbingMovement->bOrientRotationToMovement = false;

	}
	
	//AddCamera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ClimbCamera"));

	Camera->bUsePawnControlRotation = true;
	
	if (bFistPirsonView)
	{
		Camera->AttachToComponent(FistCameraSpringArm, FAttachmentTransformRules::KeepRelativeTransform, USpringArmComponent::SocketName);
		
		bUseControllerRotationYaw = true;
		ClimbingMovement->bOrientRotationToMovement = false;
			
	}
	else
	{
		Camera->AttachToComponent(ThridCameraSpringArm, FAttachmentTransformRules::KeepRelativeTransform, USpringArmComponent::SocketName);
		
		bUseControllerRotationYaw = false;
		ClimbingMovement->bOrientRotationToMovement = true;
	}
	
	
}

void AClimbingCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	OverlopObject = Cast<AOverlapObject>(OtherActor);
	if (OverlopObject)
	{
		if (ClimbingMovement->GetMode(OverlopObject->GetObjectType()).CanSetMode())
		{
			ClimbingMovement->SetClimbMode(OverlopObject->GetObjectType());
		}
		
	}
	
}



// Called when the game starts or when spawned
void AClimbingCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	ChangeView(true);
	
}

// Called every frame
void AClimbingCharacter::Tick( float DeltaTime )
{
	Super::Tick(DeltaTime);
}



// Called to bind functionality to input
void AClimbingCharacter::SetupPlayerInputComponent(class UInputComponent* _InputComponent)
{
	Super::SetupPlayerInputComponent(_InputComponent);

	_InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	_InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	_InputComponent->BindAction("QuickTurn", IE_Pressed, this, &AClimbingCharacter::QuickTurn);
	_InputComponent->BindAction("Crouch", IE_Pressed, this, &AClimbingCharacter::CrouchFunk);
	_InputComponent->BindAction("Crouch", IE_Released, this, &AClimbingCharacter::UnCrouchFunk);
	_InputComponent->BindAction("SwitchView", IE_Pressed, this, &AClimbingCharacter::SwitchView);
	_InputComponent->BindAxis("CameraPitch", this, &AClimbingCharacter::CameraPitch);
	_InputComponent->BindAxis("CameraYaw", this, &AClimbingCharacter::CameraYaw);
	_InputComponent->BindAxis("MoveForward", this, &AClimbingCharacter::MoveForward);
	_InputComponent->BindAxis("MoveRight", this, &AClimbingCharacter::MoveRight);
	
	
}




void AClimbingCharacter::MoveForward(float AxisValue)
{
	if (ClimbingMovement && (ClimbingMovement->UpdatedComponent == RootComponent))
	{
		switch (ClimbingMovement->GetCurrentClimbingMode())
		{
		case EClimbingPawnModeType::LeftWallRun:
		case EClimbingPawnModeType::RightWallRun:
			ClimbingMovement->AddInputVector(GetActorForwardVector() * AxisValue);
			break;

		case EClimbingPawnModeType::Climb:
		case EClimbingPawnModeType::InclinedSlide:
		case EClimbingPawnModeType::Slide:
			break;

		default:
			ClimbingMovement->AddInputVector(FRotator(0, GetControlRotation().Yaw, 0).Vector() * AxisValue);
			break;
		}
	}
}

void AClimbingCharacter::MoveRight(float AxisValue)
{
	if (ClimbingMovement && (ClimbingMovement->UpdatedComponent == RootComponent))
	{

		switch (ClimbingMovement->GetCurrentClimbingMode())
		{
		
		case EClimbingPawnModeType::Climb:
		case EClimbingPawnModeType::InclinedSlide:
						
			ClimbingMovement->AddInputVector(GetActorRightVector() * AxisValue);
			break;

		case EClimbingPawnModeType::LeftWallRun:
		case EClimbingPawnModeType::RightWallRun:
		case EClimbingPawnModeType::Slide:
			break;

		default:

			ClimbingMovement->AddInputVector((FRotator(0, GetControlRotation().Yaw, 0) + FRotator(0,90,0)).Vector() * AxisValue);
			break;

		}
	}
}

void AClimbingCharacter::CameraPitch(float AxisValue)
{
	AddControllerPitchInput(AxisValue);
}

void AClimbingCharacter::CameraYaw(float AxisValue)
{
	

	if (BlockYawRange)
	{
		int ActorYaw = GetPositiveAngle(GetActorRotation().Yaw);
		int ControlYaw = GetWorld()->GetFirstPlayerController()->GetControlRotation().Yaw;
		if (ActorYaw != ControlYaw)
		{
			if (GetLeftOrRight(ActorYaw, ControlYaw) == EPointLocation::Left && AxisValue < 0)
			{
				if (GetLeftOrRight(GetPositiveAngle(ActorYaw - LeftYawRange), ControlYaw) == EPointLocation::Left) return;
				
			}
				
			if (GetLeftOrRight(ActorYaw, ControlYaw) == EPointLocation::Right && AxisValue > 0)
			{
				if (GetLeftOrRight(GetPositiveAngle(ActorYaw + RightYawRange), ControlYaw) == EPointLocation::Right) return;

			}
		}

	}
	
	AddControllerYawInput(AxisValue);
	
	
}

void AClimbingCharacter::Jump()
{
	if (ClimbingMovement->GetCurrentClimbingMode() == EClimbingPawnModeType::Run)
	{
		Super::Jump();
	}
	else if (ClimbingMovement->GetCurrentClimbingMode() == EClimbingPawnModeType::LeftWallRun || ClimbingMovement->GetCurrentClimbingMode() == EClimbingPawnModeType::RightWallRun)
	{
		static_cast<FClimbingPawnModeLeftWallRun*>(&ClimbingMovement->GetMode(ClimbingMovement->GetCurrentClimbingMode()))->SetUnsetFromJump();

		ClimbingMovement->DoJump(false);
	}
	else
	{
		ClimbingMovement->DoJump(false);
	}
}

void AClimbingCharacter::CrouchFunk()
{
	
	switch (ClimbingMovement->GetCurrentClimbingMode())
	{
	case EClimbingPawnModeType::Run:
		if (ClimbingMovement->GetMode(EClimbingPawnModeType::Slide).CanSetMode())
		{
			//Crouch();
			ClimbingMovement->SetClimbMode(EClimbingPawnModeType::Slide);
		}
		else
		{
			Crouch();
		}

		break;
	
	case EClimbingPawnModeType::Climb:
	case EClimbingPawnModeType::LeftWallRun:
	case EClimbingPawnModeType::RightWallRun:
	case EClimbingPawnModeType::ZipLine:
	case EClimbingPawnModeType::LadderMove:
		
		ClimbingMovement->SetClimbMode(EClimbingPawnModeType::Run);

		break;

	}
}

void AClimbingCharacter::UnCrouchFunk()
{
	
	switch (ClimbingMovement->GetCurrentClimbingMode())
	{
	case EClimbingPawnModeType::Run:
		UnCrouch();
		break;

	case EClimbingPawnModeType::Slide:
		UnCrouch();
		ClimbingMovement->SetClimbMode(EClimbingPawnModeType::Run);
		break;
		
	}
	
}

void AClimbingCharacter::SwitchView()
{
	ChangeView(!bFistPirsonView);
}

void AClimbingCharacter::ChangeView(bool FistPirson)
{
	if (FistPirson == bFistPirsonView) return;

	Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	if (FistPirson)
	{
		Camera->AttachToComponent(FistCameraSpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("SpringEndpoint"));
		
		bFistPirsonView = true;
		
		if (ClimbingMovement->GetCurrentClimbingMode() != EClimbingPawnModeType::Climb)
		{
			bUseControllerRotationYaw = true;
			ClimbingMovement->bOrientRotationToMovement = false;
			
		}
	}
	else
	{
		Camera->AttachToComponent(ThridCameraSpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("SpringEndpoint"));

		bFistPirsonView = false;

		if (ClimbingMovement->GetCurrentClimbingMode() != EClimbingPawnModeType::Climb)
		{
			bUseControllerRotationYaw = false;
			ClimbingMovement->bOrientRotationToMovement = true;
			
		}
	}

}

float AClimbingCharacter::GetAxisValueBP(FName NameAxis)
{ 
	if (InputComponent) return InputComponent->GetAxisValue(NameAxis); 

	else return 0; 
}

void AClimbingCharacter::BlockCameraYawRangeFromCharacter(float _LeftYawRange, float _RightYawRange)
{

	LeftYawRange = _LeftYawRange;

	RightYawRange = _RightYawRange;
	
	BlockYawRange = true;
	
}

float AClimbingCharacter::GetPositiveAngle(float Angle)
{
	if (std::abs(Angle) > 360)
	{
		if (Angle < 0) Angle += 360;

		if (Angle > 0) Angle -= 360;
	}

	if (Angle < 0)
	{
		return Angle + 360;
	}
	else
	{
		return Angle;
	}
}

EPointLocation AClimbingCharacter::GetLeftOrRight(float BaseAngle, float Point)
{

	float Distance = Point - BaseAngle;

	if (Distance > 0)
	{
		if (Distance < 180) return EPointLocation::Right;
		else return EPointLocation::Left;
	}
	else
	{
		if (std::abs(Distance) < 180) return EPointLocation::Left;
		else return EPointLocation::Right;
	}
}


void AClimbingCharacter::QuickTurn()
{
	FRotator ControlRotation = GetControlRotation();

	ControlRotation.Yaw += 180;

	GetController()->SetControlRotation(ControlRotation);
}
