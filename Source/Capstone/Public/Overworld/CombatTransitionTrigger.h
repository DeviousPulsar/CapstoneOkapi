// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelTransitionHandler.h"
#include "CombatTransitionTrigger.generated.h"

UCLASS()
class CAPSTONE_API ACombatTransitionTrigger : public AActor
{
	GENERATED_BODY()
	
	ULevelTransitionHandler* TransitionHandler;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UShapeComponent* TriggerVolume;

public:	
	ACombatTransitionTrigger();

	UPROPERTY(EditAnywhere)
	FName CombatScene;

	UPROPERTY(EditAnywhere, meta=(MakeEditWidget))
	FTransform EjectDestination;

protected:
	virtual void BeginPlay() override;

private: 
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
