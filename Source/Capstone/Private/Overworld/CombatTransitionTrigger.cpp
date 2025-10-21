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

	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ACombatTransitionTrigger::OnOverlapBegin);
}

void ACombatTransitionTrigger::BeginPlay()
{
	Super::BeginPlay();

	TransitionHandler = GetGameInstance()->GetSubsystem<ULevelTransitionHandler>();
}

void ACombatTransitionTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
	FVector Dir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector ReturnPosition = EjectDistance*Dir;
	FQuat ReturnRotation = (-1*Dir).Rotation().Quaternion();
		
	TransitionHandler->LoadCombatScene(CombatScene, ReturnPosition, ReturnRotation);
}