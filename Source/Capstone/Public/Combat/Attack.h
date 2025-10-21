// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridPosition.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Attack.generated.h"

/// Struct to define one "keyframe" of an attack sequence.
/// Timing information is stored as a start time in seconds since the attack was requested and 2 deltas that
/// determine how long the warning for the attack will be up and how long the attack will deal damage repsectively.
USTRUCT(BlueprintType)
struct FAttackStage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, DisplayName="Delay Since Previous Stage")
	double Delay;

	UPROPERTY(EditAnywhere)
	double WarningLength;

	UPROPERTY(EditAnywhere)
	double DamageLength;

	UPROPERTY(EditAnywhere)
	int Damage;

	UPROPERTY(EditAnywhere)
	TArray<FGridPosition> Targets;

	/// Default Constructor - Initializes everything to 0.
	FAttackStage();
};

UCLASS(Blueprintable, BlueprintType)
class CAPSTONE_API UAttack : public UObject
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    bool bDynamic;

	UPROPERTY(EditAnywhere)
	float Cooldown;

	UPROPERTY(EditAnywhere)
	float UseTime;

	UPROPERTY(EditAnywhere)
	TArray<FAttackStage> AttackStages;
	
	/// Returns the total amount of time that the sequence will take to execute
	UFUNCTION(BlueprintCallable, BlueprintPure)
	double Length() const;

    /// Returns a static copy of attack with origin at (x,y)
    UFUNCTION(BlueprintCallable, BlueprintPure)
    UAttack* AsStaticAttack(int x, int y) const;

	// Appends the given attack's stages to the end of this attack
	// The given delay will be added between the last stage of this attack and the first stage of the given attack
	// bDynamic, UseTime and Cooldown of this attack will be preserved
	// If the given attack is dynamic it will be will not be converted to a static attack
	UFUNCTION()
	void Append(UAttack* Other, float DelayBetweenAttacks = 0);

	/// <summary>
	/// Increases the damage done with this attack by the given fixed rate
	/// </summary>
	/// <param name="Damage"></param>
	UFUNCTION(BlueprintCallable)
	void Buff(int32 Damage);
};
