// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CapstoneSaveGame.generated.h"

/**
 * UClass to handle save data
 */
UCLASS()
class CAPSTONE_API UCapstoneSaveGame : public ULocalPlayerSaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Audio Settings")
	float MusicVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Audio Settings")
	float SFXVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
	int GamePhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FVector SaveLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FQuat SaveRotation;
};
