// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WwisePersistantAudioSubsystem.generated.h"

/**
 * GameInstanceSubsystem used to keep persistant audio loaded between level loads
 * Elio! Remember to use UFUNCTION and UPROPERTY macros if you wish to expose functions and members to blueprints
 */
UCLASS()
class CAPSTONE_API UWwisePersistantAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
};
