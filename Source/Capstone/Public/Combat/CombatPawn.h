// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridPosition.h"
#include "CoreMinimal.h"
#include "BattleGrid.h"
#include "Attack.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Pawn.h"
#include "CombatPawn.generated.h"

class UAnimMontage;

UCLASS()
class CAPSTONE_API ACombatPawn : public APawn
{
	GENERATED_BODY()

protected:
	FGridPosition CurrentPosition; //tracks pawns current position on grid
	FVector StartPosition;
	FVector EndPosition;
	UPROPERTY(BlueprintReadOnly, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	int32 PawnHealth;
	bool MoveAllowed; // controls the time based movement limit to prevent spamming
	bool IsPlayer;
	ABattleGrid* Grid;
	float TimeSinceStun;
	float TimeSinceVulnerable;
	float Vulnerable;
	float MoveCooldown;
	float Defend;
	bool Parry;
	bool ParryProt;
	float TimeSinceParry;
	bool bIsFrozen;

public:
	UPROPERTY(EditAnywhere, Category = "Pawn Information")
	int32 InitialHealth;

	UPROPERTY(EditAnywhere, Category = "Pawn Information")
	float MoveStun;

	UPROPERTY(EditAnywhere, Category = "Pawn Information")
	float InvTime;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryWindow;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryCooldown;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryProtTime;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryStunTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* HitReactMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* MoveForwardMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* MoveBackwardMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* MoveLeftMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* MoveRightMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anim")
	UAnimMontage* ParryMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* HealEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* ParryEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* InvulnerableEffect;

	UPROPERTY()
	UNiagaraComponent* InvulnerableComponent;

	UPROPERTY()
	UNiagaraComponent* HealingComponent;

	UPROPERTY()
	UNiagaraComponent* ParryComponent;

	bool ParryBoost;

	/// @brief The default constructor, needed for spawning in new objects
	ACombatPawn();

	UFUNCTION(BlueprintCallable)
	bool GetIsPlayer();

	/// @brief Initializes variables, call immediately after the default constructor
	/// @param X
	/// @param Y 
	/// @param IsPlayer 
	/// @param StartingHealth 
	void Initialize(int32 X, int32 Y, ABattleGrid* BattleGrid);

	/// @brief Moves the combat pawn in the requested direction. 
	/// Checks if the pawn will move off the board.
	/// @param Vector - the input vector, converted to x and y
	UFUNCTION(BlueprintCallable)
	void Move(FVector2D Vector);

	UFUNCTION(BlueprintCallable)
	void ForceMove(FVector2D Vector);

	/// @brief Getter for the position
	/// @return FGridPosition - the current position of the pawn on the grid
	UFUNCTION(BlueprintCallable)
	FGridPosition GetPosition();

	/// @brief Getter for the CombatPawns health
	/// @return 
	UFUNCTION(BlueprintCallable)
	int32 GetHealth();

	/// @brief Changes the health of the combat pawn.
	/// @param AmountToChange positive or negative, changes the pawns current health
	/// @return the new value of health
	UFUNCTION(BlueprintCallable)
	int32 EditHealth(int32 AmountToChange);

	/// <summary>
	/// Stops the player from moving for the specified ammount of time
	/// </summary>
	/// <param name="Length"></param>
	void Stun(float Length);

	/// <summary>
	/// Sets the ammount of damage mitigated from each attack
	/// </summary>
	/// <param name="Defence"></param>
	void SetDefend(float Defence);

	/// <summary>
	/// Sets the parry field to true
	/// </summary>
	void AttemptParry();

	/// Allows or disallows the pawn from acting or taking damage
	UFUNCTION(BlueprintCallable)
	void SetMovementAllowed(bool MovementAllowed);

	UFUNCTION(BlueprintCallable, Category = "Anim")
	void PlayAttackMontage(FName Section = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Anim")
	void PlayHitReactMontage(FName Section = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Anim")
	void PlayMoveMontage(UAnimMontage* Montage, FName Section = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Anim")
	void PlayParryMontage(FName Section = NAME_None);

	UFUNCTION(BlueprintCallable)
	void ReturnToCenter();


	/// @brief Pauses effect and turns it invisible
	/// @param Effect the effect to spawn
	/// @param Duration the duration of the effect (not used currently)
	/// @param Scale the scale of the effect
	/// @return the created UNiagaraComponent*
	UFUNCTION(BlueprintCallable)
	UNiagaraComponent* SpawnEffect(UNiagaraSystem* Effect, double Duration, double Scale);

	/// @brief Restarts the effect and turns it visible
	/// @param EffectComponent
	/// @return 
	UFUNCTION(BlueprintCallable)
	void ActivateEffect(UNiagaraComponent* EffectComponent);

	/// @brief Stops effect and turns it invisible
	/// @param EffectComponent
	/// @return 
	UFUNCTION(BlueprintCallable)
	void DeactivateEffect(UNiagaraComponent* EffectComponent);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
