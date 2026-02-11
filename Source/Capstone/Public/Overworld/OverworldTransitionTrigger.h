// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTransitionHandler.h"
#include "OverworldTransitionTrigger.generated.h"

class UShapeComponent;

UCLASS()
class CAPSTONE_API AOverworldTransitionTrigger : public AActor
{
	GENERATED_BODY()

public:
	AOverworldTransitionTrigger();

	UPROPERTY(EditAnywhere)
	FName OverworldScene;

	UPROPERTY(EditAnywhere)
	FString SpawnTarget;

	UFUNCTION(BlueprintCallable)
	void BeginTransition();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UShapeComponent* TriggerVolume = nullptr;

private:
	UPROPERTY()
	ULevelTransitionHandler* TransitionHandler = nullptr;
};
