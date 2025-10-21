// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridPosition.h"
#include "CombatPawn.h"
#include "BattleGrid.h"
#include "CombatPlayer.generated.h"

UENUM(BlueprintType) // Makes it appear in Blueprints
enum class EPlayerAttacks : uint8
{
	NoAttack 		UMETA(DisplayName="No Attack"),
    TestAttack    	UMETA(DisplayName="Test Attack"),
	SimpleA			UMETA(DisplayName="Simple A")
};

UENUM(BlueprintType)
//Enum for representing the currently selected focus
enum class EFocus : uint8
{
	Default UMETA(DisplayName = "Default"),
	Attack UMETA(DisplayName = "Attack"),
	Defend UMETA(DisplayName = "Defend"),
	Heal  UMETA(DisplayName = "Heal"),
};

UCLASS()
class CAPSTONE_API ACombatPlayer : public AActor
{
	GENERATED_BODY()

	FGridPosition StartingPosition; //starting grid position
	int32 StartHealth; //starting health
	ABattleGrid* Grid; //pointer to the BattleGrid
	float TimeSinceAttack;
	bool AttackAllowed;
	float AttackCooldown;
	EFocus Focus;
	
	
public:	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACombatPawn> PawnClass;

	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	ACombatPawn* Pawn;

	UPROPERTY(BlueprintReadOnly, Category = "Variables")
	bool IsMovementAllowed;

	UPROPERTY(EditAnywhere, Category = "Variables")
	int32 ParryDamageBuff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SelectedAttack")
	EPlayerAttacks LeftClickAttack; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SelectedAttack")
	EPlayerAttacks RightClickAttack;

	UPROPERTY(EditAnywhere, Category = "Attacks")
	TMap<EPlayerAttacks, TSubclassOf<UAttack>> AttackMapping;

	UPROPERTY(EditAnywhere, Category = "Attacks")
	TArray<EPlayerAttacks> AvailablePlayerAttacks; //List of enums of available player attacks

	UPROPERTY(EditAnywhere, Category = "Variables")
	float InvTime;
	
	UPROPERTY(EditAnywhere, Category = "Focus")
	int32 DamageBuff;

	UPROPERTY(EditAnywhere, Category = "Focus")
	int32 DefenseBuff;

	UPROPERTY(EditAnywhere, Category = "Focus")
	int32 HealBuff;

	// Sets default values for this actor's properties
	ACombatPlayer();

	/// @brief Initializes variables
	/// @param x 
	/// @param y 
	/// @param startingHealth 
	/// @param grid 
	void Initialize(int32 StartingX, int32 StartingY, int32 StartingHealth, ABattleGrid* BattleGrid);

    /// @brief Returns the health of the pawn
    /// @return 
	UFUNCTION(BlueprintCallable)
    int32 GetHealth();

	/// @brief Sets if the player is allowed to move
	/// @param MovementAllowed 
	UFUNCTION(BlueprintCallable)
	void SetMovementAllowed(bool MovementAllowed);

	/// @brief Returns the availabe player attacks that the player has
	/// @return 
	UFUNCTION(BlueprintCallable)
	TArray<EPlayerAttacks> GetAvailablePlayerAttacks();

	/// @brief Adds the attack to the list of available attacks for the player
	/// @param attack 
	UFUNCTION(BlueprintCallable)
	void AddAvailablePlayerAttack(EPlayerAttacks Attack);

	/// @brief Instantiates and gives the grid the attack
	/// @param attack 
	UFUNCTION(BlueprintCallable)
	void AttackGrid(EPlayerAttacks Attack);

	/// @brief Changes the attack paired with the left mouse click
	/// @param NewAttack 
	UFUNCTION(BlueprintCallable)
	void ChangeLeftClickAttack(EPlayerAttacks NewAttack);

	/// @brief Changes the attack paired with the right mouse click
	/// @param NewAttack 
	UFUNCTION(BlueprintCallable)
	void ChangeRightClickAttack(EPlayerAttacks NewAttack);

	/// <summary>
	/// Sets player focus and makes necessary changes to facilitate that
	/// </summary>
	/// <param name="Foc"></param>
	UFUNCTION(BlueprintCallable)
	void SetBuff(EFocus Foc);

	/// <summary>
	/// Lets the combat pawn know a parry is being attempted, the logic for success or failure is located in the combat pawn
	/// </summary>
	UFUNCTION(BlueprintCallable)
	void Parry();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
