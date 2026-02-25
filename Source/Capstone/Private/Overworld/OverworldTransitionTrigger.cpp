// Fill out your copyright notice in the Description page of Project Settings.


#include "Overworld/OverworldTransitionTrigger.h"
#include "Components/BoxComponent.h"

// Sets default values
AOverworldTransitionTrigger::AOverworldTransitionTrigger()
{
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(FName("TriggerVolume"));
	RootComponent = TriggerVolume;

	if (!ensureMsgf(TriggerVolume != nullptr, TEXT("%s could not generate UBoxComponent TriggerVolume"), *GetNameSafe(this))) {
		return;
	}
}

void AOverworldTransitionTrigger::BeginPlay()
{
	Super::BeginPlay();

	TransitionHandler = Cast<ULevelTransitionHandler>(GetGameInstance());
}

void AOverworldTransitionTrigger::BeginTransition() 
{
	if (TransitionHandler != nullptr)
	{
		TransitionHandler->LoadOverworldSceneWithSpawnTarget(OverworldScene, SpawnTarget);
	}
}