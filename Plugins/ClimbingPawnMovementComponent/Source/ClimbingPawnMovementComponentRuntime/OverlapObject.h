// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once

#include "GameFramework/Actor.h"
#include "OverlapObject.generated.h"

UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API AOverlapObject : public AActor
{
	GENERATED_BODY()
public:
	enum class EClimbingPawnModeType GetObjectType() const;

	

protected:
	UPROPERTY(Category = ObjectType, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EClimbingPawnModeType ObjectType;
	
};

UCLASS()
class CLIMBINGPAWNMOVEMENTCOMPONENTRUNTIME_API AZipLine : public AOverlapObject
{
	GENERATED_BODY()
public:
	
	AZipLine();

	virtual void OnConstruction(const FTransform& Transform) override;



	/** The main skeletal mesh associated with this Character (optional sub-object). */
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
		float SplineHeight;

	//UPROPERTY(Category = ZipLine, EditAnywhere, BlueprintReadOnly)
		//TArray<class USplineMeshComponent*> AddedSplineMeshComponents;


//#if WITH_EDITORONLY_DATA
//	UPROPERTY()
//	class UArrowComponent* ArrowComponent;
//#endif

protected:
	

	void SetupSpline();

};