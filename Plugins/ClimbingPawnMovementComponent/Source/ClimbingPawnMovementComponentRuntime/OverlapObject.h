// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once

#include "GameFramework/Actor.h"
#include "ClimbingPawnMode.h"
#include "OverlapObject.generated.h"


UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API AOverlapObject : public AActor
{
	GENERATED_BODY()
public:
	virtual EClimbingPawnModeType GetObjectType() const { throw; }

	
};

UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API AZipLine : public AOverlapObject
{
	GENERATED_BODY()
public:
	
	AZipLine();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual EClimbingPawnModeType GetObjectType() const override { return EClimbingPawnModeType::ZipLine; }

public:
	UPROPERTY(Category = ZipLine, VisibleDefaultsOnly, BlueprintReadOnly)
	class UStaticMeshComponent* StartBase;

	UPROPERTY(Category = ZipLine, VisibleDefaultsOnly, BlueprintReadOnly)
	class UStaticMeshComponent* EndBase;

	UPROPERTY(Category = ZipLine, VisibleDefaultsOnly, BlueprintReadOnly)
	class USplineComponent* Spline;

	UPROPERTY(Category = ZipLine, VisibleDefaultsOnly, BlueprintReadOnly)
	USceneComponent* Pivot;

	UPROPERTY(Category = ZipLine, VisibleDefaultsOnly, BlueprintReadOnly)
	class UBoxComponent* EndBox;

	UPROPERTY(Category = ZipLine, EditAnywhere, BlueprintReadOnly)
	UStaticMesh* RopeMesh;

	UPROPERTY(Category = ZipLine, EditAnywhere, BlueprintReadOnly)
		float SplineHeight = 200;

	//UPROPERTY(Category = ZipLine, EditAnywhere, BlueprintReadOnly)
		//TArray<class USplineMeshComponent*> AddedSplineMeshComponents;


//#if WITH_EDITORONLY_DATA
//	UPROPERTY()
//	class UArrowComponent* ArrowComponent;
//#endif

private:
	
	void SetupSpline();

};

UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API ALadder : public AOverlapObject
{
	GENERATED_BODY()
public:

	ALadder();

	virtual EClimbingPawnModeType GetObjectType() const override { return EClimbingPawnModeType::LadderMove; }

	UPROPERTY(Category = Ladder, VisibleDefaultsOnly, BlueprintReadOnly)
		class UBoxComponent* LadderVolumeForward;

	UPROPERTY(Category = Ladder, VisibleDefaultsOnly, BlueprintReadOnly)
		class UBoxComponent* LadderVolumeBack;

	UPROPERTY(Category = Ladder, EditAnywhere, BlueprintReadOnly)
		float LadderHeight = 300;

	UPROPERTY(Category = Ladder, EditAnywhere, BlueprintReadOnly)
		bool ClosedUp = false;

	UPROPERTY(Category = ZipLine, VisibleDefaultsOnly, BlueprintReadOnly)
		USceneComponent* Pivot;

	

	virtual void OnConstruction(const FTransform& Transform) override;
private:

	void SetupLadderHeight();

};