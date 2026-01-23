// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTransitionHandler.h"
#include "OverworldTransitionTrigger.generated.h"

UCLASS()
class CAPSTONE_API AOverworldTransitionTrigger : public AActor
{
	GENERATED_BODY()
	
	ULevelTransitionHandler* TransitionHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UShapeComponent* TriggerVolume;

public:	
	AOverworldTransitionTrigger();

	UPROPERTY(EditAnywhere)
	FName OverworldScene;

	UPROPERTY(EditAnywhere)
	FString SpawnTarget;

protected:
	virtual void BeginPlay() override;

private: 
	UFUNCTION(BlueprintCallable)
	void BeginTransition();
};
