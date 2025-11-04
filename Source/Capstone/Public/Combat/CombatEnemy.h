// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatPawn.h"
#include "BattleGrid.h"
#include "GridPosition.h"
#include "CombatEnemy.generated.h"

class UAttack;


UCLASS()
class CAPSTONE_API ACombatEnemy : public ACombatPawn
{
	GENERATED_BODY()

	FGridPosition DefaultLocation;
	int32 DefaultHealth;
	ABattleGrid* WorldGrid;
	
public:	
	UPROPERTY(EditAnywhere, Category="Attacks")
	TArray<TSubclassOf<UAttack>> AttackClasses;

	UPROPERTY(EditAnywhere, Category = "Attacks")
	double MaxSequenceLength = 15.0;

	UPROPERTY(EditAnywhere, Category = "Attacks")
	int32 MaxSequenceParts = 10;

	UPROPERTY(EditAnywhere, Category="Attacks")
	float ParryableStageChance = 0.8f;

	// Sets default values for this actor's properties
	ACombatEnemy();

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

	bool FinishedAttack() const { return HasFinishedAttack; }

};
