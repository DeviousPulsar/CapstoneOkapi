// PatrolAICharacter.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "PatrolAICharacter.generated.h"

class USphereComponent;
class ACombatTransitionTrigger;

UCLASS()
class CAPSTONE_API APatrolAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APatrolAICharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnDetectPlayer(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	void PatrolSelectTarget();

	UPROPERTY(EditAnywhere, Category = "Patrol")
	TArray<AActor*> PatrolPoints;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float SelectInterval = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float MoveInputScale = 1.f;

	UPROPERTY(EditAnywhere, Category = "Chase")
	float ChaseRadius = 800.f;

	UPROPERTY(EditAnywhere, Category = "Chase")
	float TriggerDistance = 120.f;

	UPROPERTY(EditAnywhere, Category = "Chase")
	float LoseRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	ACombatTransitionTrigger* TransitionTrigger = nullptr;

	UPROPERTY(EditAnywhere, Category = "Trigger")
	bool bTriggerOnce = true;

	UPROPERTY(VisibleAnywhere, Category = "Trigger")
	USphereComponent* DetectSphere = nullptr;

private:
	int32 PatrolIndex = 0;
	AActor* CurrentTarget = nullptr;
	FTimerHandle PatrolTimerHandle;
	bool bHasTriggered = false;

	APawn* PlayerPawn = nullptr;
	bool bChasingPlayer = false;
};
