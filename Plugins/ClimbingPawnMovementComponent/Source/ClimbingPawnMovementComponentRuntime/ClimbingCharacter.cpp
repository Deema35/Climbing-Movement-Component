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
	ClimbMesh = Components[0];

	TArray<UCapsuleComponent*> ComponentsCaps;
	GetComponents<UCapsuleComponent>(ComponentsCaps);
	ClimbCapsule = ComponentsCaps[0];
	

	//Add Camera
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	CameraSpringArm->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	CameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 8.0f), FRotator(0.0f, 0.0f, 0.0f));
	CameraSpringArm->TargetArmLength = 300.f;
	CameraSpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ClimbCamera"));
	Camera->AttachToComponent(CameraSpringArm, FAttachmentTransformRules::KeepRelativeTransform, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = false;

	
	
}

void AClimbingCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (ClimbingMovement->GetCurrentClimbingMode() == EClimbingPawnModeType::Run && Cast<AOverlapObject>(OtherActor))
	{
		
		OverlopObject = Cast<AOverlapObject>(OtherActor);
		
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
	AddControllerYawInput(AxisValue);
}

void AClimbingCharacter::Jump()
{
	if (ClimbingMovement->GetCurrentClimbingMode() == EClimbingPawnModeType::Run)
	{
		Super::Jump();
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

	if (FistPirson)
	{
		Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Camera->AttachToComponent(ClimbMesh, FAttachmentTransformRules::KeepRelativeTransform, FName("head"));
		GetWorld()->GetFirstPlayerController()->SetControlRotation(GetActorRotation());
		Camera->SetRelativeLocation(FVector(0, 10, 0));
		bFistPirsonView = true;
		
		if (ClimbingMovement->GetCurrentClimbingMode() != EClimbingPawnModeType::Climb)
		{
			bUseControllerRotationYaw = true;
			ClimbingMovement->bOrientRotationToMovement = false;
			
		}
	}
	else
	{
		Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		Camera->AttachToComponent(CameraSpringArm, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("SpringEndpoint"));
		bFistPirsonView = false;
		if (ClimbingMovement->GetCurrentClimbingMode() != EClimbingPawnModeType::Climb)
		{
			bUseControllerRotationYaw = false;
			ClimbingMovement->bOrientRotationToMovement = true;
			
		}
	}

}