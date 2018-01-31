// Copyright 2016 - 2018 Dmitriy Pavlov

#include "OverlapObject.h"
#include "ClimbingPawnMovementComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "ClimbingPawnMode.h"

EClimbingPawnModeType AOverlapObject::GetObjectType() const
{
	return ObjectType;
}

AZipLine::AZipLine()
{
	
	ObjectType = EClimbingPawnModeType::ZipLine;
	SplineHeight = 200;

	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	Pivot->SetMobility(EComponentMobility::Static);
	RootComponent = Pivot;
	StartBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StartBase"));
	StartBase->SetupAttachment(Pivot);
	EndBase = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndBase"));
	EndBase->SetupAttachment(Pivot);
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetupAttachment(Pivot);
	EndBox = CreateDefaultSubobject<UBoxComponent>(TEXT(" EndBox"));
	EndBox->bGenerateOverlapEvents = true;
	EndBox->SetupAttachment(Pivot);
	EndBox->SetBoxExtent(FVector(30, 30, 0.5 * SplineHeight));

//#if WITH_EDITORONLY_DATA
//	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
//	ArrowComponent->SetupAttachment(Pivot);
//#endif
	

	//AddedSplineMeshComponents.Empty();
}


void AZipLine::OnConstruction(const FTransform& Transform)
{
	FVector Loc;
	FVector Tangent;

	if (!StartBase || !EndBase || !Spline || !EndBox) return;

	Spline->SetRelativeLocation(FVector(0, 0, SplineHeight));
	Spline->GetLocationAndTangentAtSplinePoint(0, Loc, Tangent, ESplineCoordinateSpace::Local);
	StartBase->SetRelativeLocation(Loc);
	Spline->GetLocationAndTangentAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, Loc, Tangent, ESplineCoordinateSpace::Local);
	EndBase->SetRelativeLocation(Loc);
	Loc.Z += 0.5 * SplineHeight;
	EndBox->SetRelativeLocation(Loc);

	SetupSpline();
	
	
}
//#if WITH_EDITORONLY_DATA
//void AZipLine::PostEditMove(bool bFinished)
//{
//	Super::PostEditMove(bFinished);
//	//SetupSpline();
//}
//#endif //WITH_EDITORONLY_DATA




void AZipLine::SetupSpline()
{
	//int32 SplineMeshComponentsNum = AddedSplineMeshComponents.Num();
	
	//remove spline mesh components to get to Spline->GetNumSplinePoints() - 1
	/*for (int32 Index = SplineMeshComponentsNum - 1; Index >= Spline->GetNumberOfSplinePoints() - 1; --Index)
	{
		AddedSplineMeshComponents[Index]->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		AddedSplineMeshComponents[Index]->UnregisterComponent();
	}*/

	//remove entries from array or make room for new components
	//AddedSplineMeshComponents.SetNum(Spline->GetNumberOfSplinePoints() - 1, true);

	//construct spline mesh components to get to Spline->GetNumSplinePoints() - 1
	//previously constructed components will remain in the AddedSplineMeshComponents array unchanged
	for (int32 Index = 0; Index < Spline->GetNumberOfSplinePoints() - 1; Index++)
	{
		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		


		FVector pointLocationStart, pointTangentStart, pointLocationEnd, pointTangentEnd;

		Spline->GetLocalLocationAndTangentAtSplinePoint(Index, pointLocationStart, pointTangentStart);
		Spline->GetLocalLocationAndTangentAtSplinePoint(Index + 1, pointLocationEnd, pointTangentEnd);

		SplineMesh->SetStartAndEnd(pointLocationStart, pointTangentStart, pointLocationEnd, pointTangentEnd);

		if (RopeMesh)
		{
			SplineMesh->SetStaticMesh(RopeMesh);
		}

		SplineMesh->SetCollisionProfileName(FName("OverlapOnlyPawn"));
		SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		SplineMesh->SetMobility(EComponentMobility::Movable);
		SplineMesh->SetupAttachment(Spline);

		SplineMesh->bCastDynamicShadow = false;
	}
	
	
	RegisterAllComponents();

	
}

