// Copyright 2016 - 2018 Dmitriy Pavlov

#pragma once
#include "CoreMinimal.h"
#include <map>
#include <vector>
#include <memory>
#include <functional>

class AActor;
struct FHitResult;

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

template <typename T, typename Tid>
class TClimbingModeStorage
{
public:

	T& Get(Tid ComponentType);

	void IniciateComponents(std::vector<Tid> ComponentsForCreate, const std::function<T* (Tid ComponentID)> ComponentCreater);

private:

	std::map<Tid, std::shared_ptr<T>> LevelComponents;
};


