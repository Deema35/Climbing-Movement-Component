// Copyright 2016 Dmitriy

#pragma once

#include "GameFramework/Actor.h"
#include "OverlapObject.generated.h"

UCLASS()
class CLIMBINGSYSTEM_API AOverlapObject : public AActor
{
	GENERATED_BODY()
public:
	enum EClimbingMode GetObjectType() const;

	

protected:
	UPROPERTY(Category = ObjectType, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<enum EClimbingMode> ObjectType;
	
};

UCLASS()
class CLIMBINGSYSTEM_API AZipLine : public AOverlapObject
{
	GENERATED_BODY()
public:
	
	AZipLine();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostEditMove(bool bFinished) override;

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
	UBoxComponent* EndBox;

	UPROPERTY(Category = ZipLine, EditAnywhere, BlueprintReadOnly)
	UStaticMesh* RopeMesh;

	UPROPERTY(Category = ZipLine, EditAnywhere, BlueprintReadOnly)
		float SplineHeight;


#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UArrowComponent* ArrowComponent;
#endif

protected:
	TArray<class USplineMeshComponent*> AddedSplineMeshComponents;

	void SetupSpline();

};