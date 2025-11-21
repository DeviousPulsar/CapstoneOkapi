// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridPosition.h"

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Override Warning Effect"))
	UNiagaraSystem* TargetWarningEffect;

	UPROPERTY(EditAnywhere, BluePrintReadWrite, meta = (DisplayName = "Override Attack Effect"))
	UNiagaraSystem* TargetAttackEffect;

	UPROPERTY(EditAnywhere)
	bool bParriable = true;

	/// Default Constructor - Initializes everything to 0.
	FAttackStage();
};

UCLASS(Blueprintable, BlueprintType)
class CAPSTONE_API UAttack : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bDefaultParryable = true;

    UPROPERTY(EditAnywhere)
    bool bDynamic;

	UPROPERTY(EditAnywhere)
	float Cooldown;

	UPROPERTY(EditAnywhere)
	float UseTime;

	UPROPERTY(EditAnywhere)
	TArray<FAttackStage> AttackStages;

	// VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* WarningEffect;

	UPROPERTY(EditAnywhere, BluePrintReadWrite)
	UNiagaraSystem* AttackEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* UnparryableWarningEffect;

	UPROPERTY(EditAnywhere, BluePrintReadWrite)
	UNiagaraSystem* UnparryableAttackEffect;

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
	void Buff(float Damage);
};

// Used for 
USTRUCT(BlueprintType)
struct FAttackEffect
{
    GENERATED_BODY()
public:
    FAttackEffect() : Effect(nullptr), Duration(1), Scale(1){
		// empty
	}

    FAttackEffect(UNiagaraSystem* InEffect, double InDuration, double InScale) : Effect(InEffect), Duration(InDuration), Scale(InScale){
		// empty
	}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* Effect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    double Scale;
};