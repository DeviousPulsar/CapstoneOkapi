// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CapstoneSaveGame.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayerAction, Log, All);

/**
 * UClass to handle save data
 */
UCLASS()
class CAPSTONE_API UCapstoneSaveGame : public USaveGame
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
	int BattlesEntered;

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
	int BattlesWon;

	UPROPERTY(VisibleAnywhere, Category = "Gameplay")
	TSet<FName> ConsumedTriggers;

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Audio Settings")
	float MusicVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Audio Settings")
	float SFXVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FName CurrentLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FVector SaveLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FQuat SaveRotation;

	UFUNCTION(BlueprintCallable)
	void Initialize ();

	UFUNCTION(BlueprintCallable)
	void Reset();

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

	UFUNCTION(BlueprintCallable)
	bool IsConsumed(FName Id) const;

	UFUNCTION(BlueprintCallable)
	void Consume(FName Id);
};
