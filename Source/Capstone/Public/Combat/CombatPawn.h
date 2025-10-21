// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GridPosition.h"
#include "CoreMinimal.h"
#include "BattleGrid.h"
#include "GameFramework/Pawn.h"
#include "CombatPawn.generated.h"


UCLASS()
class CAPSTONE_API ACombatPawn : public APawn
{
	GENERATED_BODY()
	FGridPosition CurrentPosition; //tracks pawns current position on grid
	UPROPERTY(BlueprintReadOnly, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	int32 InitialHealth;
	int32 PawnHealth;
	bool MoveAllowed; // controls the time based movement limit to prevent spamming
	bool IsPlayer;
	ABattleGrid* Grid;
	float TimeSinceStun;
	float TimeSinceVulnerable;
	float InvTime;
	float Vulnerable;
	float MoveCooldown;
	int32 Defend;
	bool Parry;
	bool ParryProt;
	float TimeSinceParry;
	
public:

	UPROPERTY(EditAnywhere, Category = "Variables")
	float MoveStun;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryWindow;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryCooldown;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryProtTime;

	UPROPERTY(EditAnywhere, Category = "Parry")
	float ParryStunTime;

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
	void Initialize(int32 X, int32 Y, bool B_IsPlayer, int32 StartingHealth, ABattleGrid* BattleGrid, float InvulTime);

	/// @brief Moves the combat pawn in the requested direction. 
	/// Checks if the pawn will move off the board.
	/// @param Vector - the input vector, converted to x and y
	UFUNCTION(BlueprintCallable)
	void Move(FVector2D Vector);

	/// @brief Getter for the position
	/// @return FGridPosition - the current position of the pawn on the grid
	FGridPosition GetPosition();

	/// @brief Getter for the CombatPawns health
	/// @return 
	int32 GetHealth();

	/// @brief Changes the health of the combat pawn.
	/// @param AmountToChange positive or negative, changes the pawns current health
	/// @return the new value of health
	int32 EditHealth(int32 AmountToChange);

	/// <summary>
	/// Stops the player from moving for the specified ammount of time
	/// </summary>
	/// <param name="Length"></param>
	void Stun(float Length);

	/// <summary>
	/// Sets the ammount of damage mitigated from each attack
	/// </summary>
	/// <param name="DamageBlocked"></param>
	void SetDefend(int32 DamageBlocked);

	/// <summary>
	/// Sets the parry field to true
	/// </summary>
	void AttemptParry();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	//virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
