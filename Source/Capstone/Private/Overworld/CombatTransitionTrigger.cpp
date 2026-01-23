// Fill out your copyright notice in the Description page of Project Settings.


#include "Overworld/CombatTransitionTrigger.h"
#include "Components/BoxComponent.h"

// Sets default values
ACombatTransitionTrigger::ACombatTransitionTrigger()
{
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(FName("TriggerVolume"));
	RootComponent = TriggerVolume;

	if (!ensureMsgf(TriggerVolume != nullptr, TEXT("%s could not generate UBoxComponent TriggerVolume"), *GetNameSafe(this))) {
		return;
	}
}

void ACombatTransitionTrigger::BeginPlay()
{
	Super::BeginPlay();

	TransitionHandler = Cast<ULevelTransitionHandler>(GetGameInstance());
}

void ACombatTransitionTrigger::BeginTransition() 
{
	FTransform XformedDest = ActorToWorld()*EjectDestination;
	
	if (TransitionHandler != nullptr)
	{
		TransitionHandler->LoadCombatScene(CombatScene, XformedDest.GetLocation(), XformedDest.GetRotation());
	}
}