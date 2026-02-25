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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Persist")
	FName PersistentId;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Patrol")
	AActor* PatrolAreaActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float FallbackRadius = 600.f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float CruiseSpeed = 220.f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float ChaseSpeed = 260.f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float AcceptanceRadius = 35.f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float ReachedHoldTime = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float RetargetCooldownTime = 0.20f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float StuckTimeThreshold = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Move")
	float MinMovedDistance = 0.5f;

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
	bool bHasTriggered = false;

	APawn* PlayerPawn = nullptr;
	bool bChasingPlayer = false;

	FVector SpawnLocation = FVector::ZeroVector;
	FVector CruiseTarget = FVector::ZeroVector;
	bool bHasCruiseTarget = false;

	float RetargetCooldown = 0.f;
	float ReachedHold = 0.f;
	float StuckTime = 0.f;

	FVector PickPointInPatrolArea() const;
	void RefreshCruiseTarget();
	bool MoveConstantSpeedToward(const FVector& Target, float Speed, float DeltaSeconds);
};