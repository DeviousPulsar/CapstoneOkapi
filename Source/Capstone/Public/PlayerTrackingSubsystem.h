// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerTrackingSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerAction, Log, All);

/**
 * GameInstantceSubSystem used to track information about the Player's flow through the game 
 */
UCLASS()
class CAPSTONE_API UPlayerTrackingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	UPROPERTY()
	int BattlesEntered;

	UPROPERTY()
	int BattlesWon;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void LogBattleEntered();

	UFUNCTION(BlueprintCallable)
	void LogBattleWin();

	UFUNCTION(BlueprintPure)
	int GetBattlesEntered() const;

	UFUNCTION(BlueprintPure)
	int GetBattlesWon() const;

	UFUNCTION(BlueprintPure)
	int GetBattlesLost() const;
};
