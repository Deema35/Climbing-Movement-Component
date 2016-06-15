// Copyright 2016 Dmitriy

#include "ClimbingSystem.h"
#include "ClimbingCharacter.h"
#include "ClimbingPawnMovementComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "OverlapObject.h"


 




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
	CameraSpringArm->AttachTo(RootComponent);
	CameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 8.0f), FRotator(0.0f, 0.0f, 0.0f));
	CameraSpringArm->TargetArmLength = 300.f;
	CameraSpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ClimbCamera"));
	Camera->AttachTo(CameraSpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = true;
	bUseControllerRotationYaw = false;

	
	
}

void AClimbingCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (ClimbingMovement->GetClimbingMode() == EClimbingMode::CLIMB_None && Cast<AOverlapObject>(OtherActor))
	{
		
		OverlopObject = Cast<AOverlapObject>(OtherActor);
		
		if (ClimbingMovement->CanSetClimbMode(OverlopObject->GetObjectType()))
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
void AClimbingCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AClimbingCharacter::CrouchFunk);
	InputComponent->BindAction("Crouch", IE_Released, this, &AClimbingCharacter::UnCrouchFunk);
	InputComponent->BindAction("SwitchView", IE_Pressed, this, &AClimbingCharacter::SwitchView);
	InputComponent->BindAxis("CameraPitch", this, &AClimbingCharacter::CameraPitch);
	InputComponent->BindAxis("CameraYaw", this, &AClimbingCharacter::CameraYaw);
	InputComponent->BindAxis("MoveForward", this, &AClimbingCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AClimbingCharacter::MoveRight);
	
}




void AClimbingCharacter::MoveForward(float AxisValue)
{
	if (ClimbingMovement && (ClimbingMovement->UpdatedComponent == RootComponent))
	{
		switch (ClimbingMovement->GetClimbingMode())
		{
		
		
		case EClimbingMode::CLIMB_LeftWallRun:
		case EClimbingMode::CLIMB_RightWallRun:
			ClimbingMovement->AddInputVector(GetActorForwardVector() * AxisValue);
			break;

		case EClimbingMode::CLIMB_Climb:
		case EClimbingMode::CLIMB_InclinedSlide:
		case EClimbingMode::CLIMB_Slide:
			break;

		default:
			ClimbingMovement->AddInputVector(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)) * AxisValue);
			break;
		}
	}
}

void AClimbingCharacter::MoveRight(float AxisValue)
{
	if (ClimbingMovement && (ClimbingMovement->UpdatedComponent == RootComponent))
	{

		switch (ClimbingMovement->GetClimbingMode())
		{
		
		case EClimbingMode::CLIMB_Climb:
		case EClimbingMode::CLIMB_InclinedSlide:
						
			ClimbingMovement->AddInputVector(GetActorRightVector() * AxisValue);
			break;

		case EClimbingMode::CLIMB_LeftWallRun:
		case EClimbingMode::CLIMB_RightWallRun:
		case EClimbingMode::CLIMB_Slide:
			break;

		default:
			ClimbingMovement->AddInputVector(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)) * AxisValue);
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
	if (ClimbingMovement->GetClimbingMode() == EClimbingMode::CLIMB_None)
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
	
	switch (ClimbingMovement->GetClimbingMode())
	{
	case EClimbingMode::CLIMB_None:
		if (ClimbingMovement->CanSetClimbMode(EClimbingMode::CLIMB_Slide))
		{
			//Crouch();
			ClimbingMovement->SetClimbMode(EClimbingMode::CLIMB_Slide);
		}
		else
		{
			Crouch();
		}

		break;
	
	case EClimbingMode::CLIMB_Climb:
	case EClimbingMode::CLIMB_LeftWallRun:
	case EClimbingMode::CLIMB_RightWallRun:
	case EClimbingMode::CLIMB_ZipLine:
		
		ClimbingMovement->SetClimbMode(EClimbingMode::CLIMB_None);

		break;

	}
}

void AClimbingCharacter::UnCrouchFunk()
{
	
	switch (ClimbingMovement->GetClimbingMode())
	{
	case EClimbingMode::CLIMB_None:
		UnCrouch();
		break;
	case EClimbingMode::CLIMB_Slide:
		UnCrouch();
		ClimbingMovement->SetClimbMode(EClimbingMode::CLIMB_None);
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
		Camera->DetachFromParent();
		Camera->AttachTo(ClimbMesh, FName("head"), EAttachLocation::KeepRelativeOffset);
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetControlRotation(GetActorRotation());
		Camera->SetRelativeLocation(FVector(0, 10, 0));
		bFistPirsonView = true;
		
		if (ClimbingMovement->GetClimbingMode() != EClimbingMode::CLIMB_Climb)
		{
			bUseControllerRotationYaw = true;
			ClimbingMovement->bOrientRotationToMovement = false;
			
		}
	}
	else
	{
		Camera->DetachFromParent();
		Camera->AttachTo(CameraSpringArm, FName("SpringEndpoint"), EAttachLocation::KeepRelativeOffset);
		bFistPirsonView = false;
		if (ClimbingMovement->GetClimbingMode() != EClimbingMode::CLIMB_Climb)
		{
			bUseControllerRotationYaw = false;
			ClimbingMovement->bOrientRotationToMovement = true;
			
		}
	}

}