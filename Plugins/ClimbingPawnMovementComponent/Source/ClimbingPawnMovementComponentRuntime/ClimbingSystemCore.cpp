// Copyright 2016 - 2018 Dmitriy Pavlov

#include "ClimbingSystemCore.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ClimbingPawnMode.h"

bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, EDrawDebugTraceType DrawDebugType, bool ReturnPhysMat)
{
	FHitResult HitOut;
	return TraceLine(World, ActorToIgnore, Start, End, HitOut, DrawDebugType, ReturnPhysMat);
}
bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, FHitResult& HitOut, EDrawDebugTraceType DrawDebugType, bool ReturnPhysMat)
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
		ECC_Pawn, //collision channel
		TraceParams
	);

	if (DrawDebugType == EDrawDebugTraceType::ForOneFrame)
	{
		DrawDebugLine(World, Start, HitOut.bBlockingHit ? HitOut.Location : End, FColor::Red, false, -1.f, SDPG_World, 2.f);
		if (HitOut.bBlockingHit)
		{
			DrawDebugPoint(World, HitOut.Location, 20.f, FColor::Red, false, -1.f, SDPG_World);
			DrawDebugLine(World, HitOut.Location, End, FColor::Green, false, -1.f, SDPG_World, 2.f);
		}
	}
	else if (DrawDebugType == EDrawDebugTraceType::ForDuration)
	{
		float DrawTime = 2.f;
		DrawDebugLine(World, Start, HitOut.bBlockingHit ? HitOut.Location : End, FColor::Red, false, DrawTime, SDPG_World, 2.f);
		if (HitOut.bBlockingHit)
		{
			DrawDebugPoint(World, HitOut.Location, 20.f, FColor::Red, false, DrawTime, SDPG_World);
			DrawDebugLine(World, HitOut.Location, End, FColor::Green, false, DrawTime, SDPG_World, 2.f);
		}
	}
	else if (DrawDebugType == EDrawDebugTraceType::Persistent)
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

float VectorXYAngle(FVector V1, FVector V2)
{
	float V1Len = V1.Size2D();
	float V2Len = V2.Size2D();

	float ScalarProduct = (V1.X * V2.X + V1.Y * V2.Y);
	float CosVek = ScalarProduct / (V1Len * V2Len);
	float B = V1.X * V2.Y - V2.X * V1.Y;
	float Sign = B > 0 ? 1 : -1;
	return acos(CosVek)  * 180.0 / PI * Sign;
}


//***********************************************
//TLevelBilderComponentStorage
//***********************************************
TClimbingModeStorage::TClimbingModeStorage(UClimbingPawnMovementComponent& _OwningClimbiingComponent) : OwningClimbiingComponent(_OwningClimbiingComponent) {}

FClimbingPawnModeBase& TClimbingModeStorage::Get(EClimbingPawnModeType ComponentType)
{
	if (LevelComponents.empty())
	{
		IniciateComponents();
	}

	return *LevelComponents[ComponentType].get();
}


void TClimbingModeStorage::IniciateComponents()
{

	for (int i = 0; i < (int)EClimbingPawnModeType::end; i++)
	{
		FClimbingPawnModeBase* NewComponentPointer = EClimbingPawnModeTypeCreate((EClimbingPawnModeType)i, OwningClimbiingComponent);
		if (NewComponentPointer)
		{
			
			LevelComponents.insert(std::make_pair((EClimbingPawnModeType)i, std::unique_ptr<FClimbingPawnModeBase>(NewComponentPointer)));
		}
	}
}