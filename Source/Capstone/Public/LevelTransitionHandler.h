// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelTransitionHandler.generated.h"

/** A GameInstanceSubsystem that handles transitioning between Levels
 */
UCLASS(Blueprintable)
class CAPSTONE_API ULevelTransitionHandler : public UGameInstance
{
	GENERATED_BODY()

	bool bInCombat;

	FName OverworldMap;
	FVector ReturnPosition;
	FQuat ReturnRotation;
	
public:
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;
  	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

	UFUNCTION(BlueprintCallable)
	void LoadCombatScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation);

	UFUNCTION(BlueprintCallable)
	void LoadOverworldScene(const FName LevelToLoad, const FVector DestinationPosition, const FQuat DestinationRotation);

	UFUNCTION(BlueprintCallable)
	void ReturnToOverworld();

	ULevelTransitionHandler(const FObjectInitializer& ObjectInitializer);
};