// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/Actor.h"
#include "GridPosition.h"
#include "CombatPawn.h"
#include "BattleGrid.h"
#include "CombatPlayer.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UENUM(BlueprintType) // Makes it appear in Blueprints
enum class EPlayerAttacks : uint8
{
	NoAttack 		UMETA(DisplayName="No Attack"),
    TestAttack    	UMETA(DisplayName="Test Attack"),
	DreamBeam		UMETA(DisplayName="Dream Beam"),
	CrossBeam		UMETA(DisplayName = "Cross Beam")
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
class CAPSTONE_API ACombatPlayer : public ACombatPawn
{
	GENERATED_BODY()

	float TimeSinceAttack;
	bool AttackAllowed;
	float AttackCooldown;
	EFocus Focus;

protected:
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ParryAction;

public:	
	UPROPERTY(EditAnywhere, Category = "Parry")
	int32 ParryDamageBuff;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
	EPlayerAttacks LeftClickAttack; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attacks")
	EPlayerAttacks RightClickAttack;

	UPROPERTY(EditAnywhere, Category = "Attacks")
	TMap<EPlayerAttacks, TSubclassOf<UAttack>> AttackMapping;

	UPROPERTY(EditAnywhere, Category = "Attacks")
	TArray<EPlayerAttacks> AvailablePlayerAttacks; //List of enums of available player attacks
	
	UPROPERTY(EditAnywhere, Category = "Focus")
	int32 DamageBuff;

	UPROPERTY(EditAnywhere, Category = "Focus")
	int32 DefenseBuff;

	UPROPERTY(EditAnywhere, Category = "Focus")
	int32 HealBuff;

	// Sets default values for this actor's properties
	ACombatPlayer();

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

protected:
	// Called when the game starts or when spawned
	virtual void Restart() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Called on receving movement input
	void Move(const FInputActionValue& Value);
	void Attack();
	void Parry();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
