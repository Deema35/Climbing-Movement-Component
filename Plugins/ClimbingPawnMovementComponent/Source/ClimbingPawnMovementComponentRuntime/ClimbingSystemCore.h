// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once
#include "CoreMinimal.h"
#include <map>
#include <vector>
#include <memory>
#include <functional>

class AActor;
struct FHitResult;
class FClimbingPawnModeBase;
class UClimbingPawnMovementComponent;
enum class EClimbingPawnModeType : uint8;

enum class EDrawDebugTraceType : uint8
{
	None,
	ForOneFrame,
	ForDuration,
	Persistent
};


bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, FHitResult& HitOut, EDrawDebugTraceType DrawDebugType = EDrawDebugTraceType::None, bool ReturnPhysMat = false);

bool TraceLine(UWorld* World, AActor* ActorToIgnore, const FVector& Start, const FVector& End, EDrawDebugTraceType DrawDebugType = EDrawDebugTraceType::None, bool ReturnPhysMat = false);

float VectorXYAngle(FVector V1, FVector V2);


class TClimbingModeStorage
{
public:

	TClimbingModeStorage(UClimbingPawnMovementComponent& _OwningClimbiingComponent);

	FClimbingPawnModeBase& Get(EClimbingPawnModeType ComponentType);

	const FClimbingPawnModeBase& Get(EClimbingPawnModeType ComponentType) const;


private:

	std::vector<std::unique_ptr<FClimbingPawnModeBase>> LevelComponents;

	UClimbingPawnMovementComponent& OwningClimbiingComponent;
};


