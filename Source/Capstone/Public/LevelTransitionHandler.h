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

	FName OverworldMap;
	FVector ReturnPosition;
	FQuat ReturnRotation;

	static UGameInstance* GetGameInstance()
	static UWorld* GetCurrentWorld();
	
public:
	//~ Begin USubsystem interface
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; };
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem interface

	UFUNCTION(BlueprintCallable)
	void LoadCombatScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation);

	UFUNCTION(BlueprintCallable)
	void LoadOverworldScene(const FName LevelToLoad, const FVector DestinationPosition, const FQuat DestinationRotation);

	UFUNCTION(BlueprintCallable)
	void ReturnToOverworld();
};