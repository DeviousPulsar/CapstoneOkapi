// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatPawn.h"
#include "BattleGrid.h"
#include "GridPosition.h"
#include "EnemyController.generated.h"

class UAttack;


UCLASS()
class CAPSTONE_API AEnemyController : public AActor
{
	GENERATED_BODY()

	FGridPosition DefaultLocation;
	int32 DefaultHealth;
	ABattleGrid* WorldGrid;
	ACombatPawn* EnemyPawn;
	
public:	
	UPROPERTY(EditAnywhere, Category="Enemy Information")
	TSubclassOf<ACombatPawn> PawnClass;

	UPROPERTY(EditAnywhere, Category="Enemy Information")
	TArray<TSubclassOf<UAttack>> AttackClasses;

	UPROPERTY(EditAnywhere, Category = "Enemy Information")
	float InvTime;

	UPROPERTY(EditAnywhere, Category = "Enemy Attacks")
	double MaxSequenceLength = 15.0;

	UPROPERTY(EditAnywhere, Category = "Enemy Attacks")
	int32 MaxSequenceParts = 10;

	UPROPERTY(EditAnywhere, Category="Enemy Attacks")
	float ParryableStageChance = 0.8f;

		// Sets default values for this actor's properties
	AEnemyController();

	void Initialize(int32 X, int32 Y, int32 DefHealth, ABattleGrid* Grid);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	bool HasFinishedAttack = true;
	FTimerHandle AttackFinishTimer;

	void OnAttackFinished();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	float BeginEnemyAttack();

	// Moves enemy randomly on grid
	void MoveRandomOnGrid();
	UFUNCTION(BlueprintCallable) int32 GetHealth();

	


	bool FinishedAttack() const { return HasFinishedAttack; }

};
