// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TransitionScreen.h"
#include "LevelTransitionHandler.generated.h"

UENUM()
enum ELevelTransitionType
{
	NONE,
	TRANSFORM,
	TARGET,
};

UCLASS(Blueprintable)
class CAPSTONE_API ATransitionTarget : public AActor 
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	FString TransitionName;

public:
	UFUNCTION(BlueprintCallable)
	FString GetTransitionName() { return TransitionName; }
};

/** A GameInstanceSubsystem that handles transitioning between Levels
 */
UCLASS(Blueprintable)
class CAPSTONE_API ULevelTransitionHandler : public UGameInstance
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TSubclassOf<UTransitionScreen> TransitionScreenClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float LoadDelay;

	UPROPERTY()
	UTransitionScreen* TransitionScreen;

	ELevelTransitionType TransitionType;

	FName OverworldMap;
	FString EjectTarget;
	FVector ReturnPosition;
	FQuat ReturnRotation;

	bool bLoadScheduled;
	FName MapScheduledToLoad;
	FTimerHandle TimerHandle;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName ToConsumeOnCombatWin;

	virtual void Init();
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;
  	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

	UFUNCTION()
	void LoadScene(const FName LevelToLoad, bool bToSchedule = true);

	UFUNCTION()
	void ScheduleLoad(FName LevelToLoad);

	UFUNCTION()
	void LoadScheduled();

	UFUNCTION(BlueprintPure)
	bool IsLoadScheduled() const { return bLoadScheduled; }

	UFUNCTION(BlueprintCallable)
	void LoadCombatScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation);
	
	UFUNCTION(BlueprintCallable)
	void LoadCombatSceneWithReturnTarget(const FName LevelToLoad, const FString pEjectTarget);

	UFUNCTION(BlueprintCallable)
	void LoadOverworldScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation);

	UFUNCTION(BlueprintCallable)
	void LoadOverworldSceneWithSpawnTarget(const FName LevelToLoad, const FString pEjectTarget);

	UFUNCTION(BlueprintCallable)
	void ReturnToOverworld();

	UFUNCTION(BlueprintCallable)
	void ReloadCombatScene();

	UFUNCTION(BlueprintPure)
	FTransform GetRespawnTransform() const;

	ULevelTransitionHandler(const FObjectInitializer& ObjectInitializer);
};
