// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "BattleGrid.h"
#include "CombatPlayer.h"
#include "CombatEnemy.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"

/**
* Agnostically designates the player and enemy regardless of the particular controller being used
*/
UENUM()
enum ETarget
{
	EPlayer = 0,
	EEnemy = 1
};

UCLASS()
class CAPSTONE_API ACombatManager : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DialogueSupport") bool bDialogueEnabled;
	UPROPERTY(BlueprintReadOnly) ACombatPlayer* PlayerC;
	UPROPERTY(BlueprintReadOnly) ACombatEnemy* EnemyC;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ABattleGrid> GridClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACombatEnemy> EnemyClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACombatPlayer> PlayerClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueSupport") UDataTable* DialogueTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DialogueSupport") int32 EncounterNum;

	ABattleGrid* Grid;


	/**
	* Creates a new combat manager object that controls the turn cycle, tracks player and enemy health
	* and detects win/loss states
	*/
	ACombatManager();

	/**
	* Determines if a win or loss state has occurred and signals the appropriate action if one has been reached
	*/
	UFUNCTION(BlueprintCallable)
	void AssessWLState();

	/**
	* Begins a turn cycle by triggering the menu and accepting the player's input then
	* signalling to enemy that it can begin attacking
	*/
	UFUNCTION(BlueprintCallable)
	void BeginTurnCycle();


protected:
	UFUNCTION(BlueprintCallable)
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void PlayerWins();
	void PlayerLoses();
	void TransitionToOverworld();

	UFUNCTION(BlueprintCallable) void AssignControllers();
};
