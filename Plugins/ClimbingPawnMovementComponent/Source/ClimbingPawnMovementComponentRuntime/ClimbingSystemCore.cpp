// Copyright 2016 - 2018 Dmitriy Pavlov

#include "ClimbingSystemCore.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"


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
	float V1Len = pow(pow(V1.X, 2) + pow(V1.Y, 2), 0.5);
	float V2Len = pow(pow(V2.X, 2) + pow(V2.Y, 2), 0.5);

	float ScalarProduct = (V1.X * V2.X + V1.Y * V2.Y);
	float CosVek = ScalarProduct / (V1Len * V2Len);
	float B = V1.X * V2.Y - V2.X * V1.Y;
	float Sign = B > 0 ? 1 : -1;
	return acos(CosVek)  * 180.0 / PI * Sign;
}


//***********************************************
//TLevelBilderComponentStorage
//***********************************************

template <typename T, typename Tid>
T& TClimbingModeStorage<T, Tid>::Get(Tid ComponentType)
{
	if (LevelComponents.empty())
	{
		throw FString("Component storage is empty");
	}

	return *LevelComponents[ComponentType].get();
}

template <typename T, typename Tid>
void TClimbingModeStorage<T, Tid>::IniciateComponents(std::vector <Tid> ComponentsForCreate, const std::function<T* (Tid ComponentID)> ComponentCreater)
{
	LevelComponents.clear();

	for (int i = 0; i < ComponentsForCreate.size(); i++)
	{
		T* NewComponentPointer = ComponentCreater(ComponentsForCreate[i]);
		if (NewComponentPointer)
		{
			std::shared_ptr<T> NewComponent(NewComponentPointer);
			LevelComponents.insert(std::make_pair(ComponentsForCreate[i], NewComponent));
		}
	}
}