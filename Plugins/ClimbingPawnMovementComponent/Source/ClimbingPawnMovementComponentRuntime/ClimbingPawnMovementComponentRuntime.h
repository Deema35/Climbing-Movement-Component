// Copyright 2016 Dmitriy

#pragma once

#include "EngineMinimal.h"
#include "ModuleManager.h"




class FClimbingPawnMovementComponentRuntime : public IModuleInterface
{

public:
	virtual void StartupModule() override { UE_LOG(LogTemp, Log, TEXT("Start ClimbingPawnMovementComponent")); }
	virtual void ShutdownModule() override { UE_LOG(LogTemp, Log, TEXT("End ClimbingPawnMovementComponent")); }
};
