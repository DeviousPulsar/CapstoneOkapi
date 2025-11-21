// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatPawn.h"
#include "BattleGrid.h"
#include "GridPosition.h"
#include "CombatEnemy.generated.h"

class UAttackSequence;

UCLASS()
class CAPSTONE_API ACombatEnemy : public ACombatPawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Attacks")
	TArray<TSubclassOf<UAttackSequence>> AttackSequences;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MovementCooldown = 1.0;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsMoving = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsAttacking = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsHitReact = false;

	UPROPERTY(BlueprintReadWrite, Category = "Animation")
	bool bIsParrying = false;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float AttackAnimDurationOverride = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float HitReactDuration = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float ParryDuration = 0.3f;

	// Sets default values for this actor's properties
	ACombatEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	bool HasFinishedAttack = true;
	FTimerHandle AttackFinishTimer;

	UPROPERTY()
	UAttackSequence* CurrentSequence;
	int32 CurrentSeqeunceIndex = -1;

	void OnAttackFinished();

	FTimerHandle AttackAnimTimer;
	FTimerHandle HitReactTimer;
	FTimerHandle ParryTimer;

	void OnAttackAnimFinished();
	void OnHitReactFinished();
	void OnParryFinished();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	float BeginEnemyAttack();

	// Moves enemy randomly on grid
	void MoveRandomOnGrid();

	bool FinishedAttack() const { return HasFinishedAttack; }

	UFUNCTION(BlueprintCallable)
	float BeginEnemyAttack_Anim();

	UFUNCTION(BlueprintCallable)
	void MoveRandomOnGrid_Anim();

	UFUNCTION(BlueprintCallable)
	void PlayHitReact();

	UFUNCTION(BlueprintCallable)
	void BeginParry();
};
