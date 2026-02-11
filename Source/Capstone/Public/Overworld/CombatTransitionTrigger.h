// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTransitionHandler.h"
#include "CombatTransitionTrigger.generated.h"

class UShapeComponent;

UCLASS()
class CAPSTONE_API ACombatTransitionTrigger : public AActor
{
	GENERATED_BODY()

public:
	ACombatTransitionTrigger();

	UPROPERTY(EditAnywhere)
	FName CombatScene;

	UPROPERTY(EditAnywhere, meta = (MakeEditWidget))
	FTransform EjectDestination;

	UFUNCTION(BlueprintCallable)
	void BeginTransition();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShapeComponent* TriggerVolume;

private:
	UPROPERTY()
	ULevelTransitionHandler* TransitionHandler;
};
