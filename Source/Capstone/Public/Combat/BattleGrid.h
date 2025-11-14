// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Attack.h"
#include "BattleGrid.generated.h"

/// <summary>
/// Class to represent the grid battles take place on. Can be given an attack object
/// and the grid will visually execute it.
/// </summary>
UCLASS()
class CAPSTONE_API ABattleGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABattleGrid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/// <summary>
	/// Returns the ammount of damage the specified tile is currently dealing
	/// </summary>
	/// <param name="Pos"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid")
	int32 DamageAtTile(FGridPosition Pos);

	UFUNCTION(BlueprintCallable, Category="Grid")
	bool IsParriableAtTile(FGridPosition Pos);

	/// <summary>
	/// Returns an FVector specifying where the specified tile is in engine
	/// </summary>
	/// <param name="Pos"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector GetTilePos(FGridPosition Pos);

	/// <summary>
	/// Returns the width of the grid
	/// </summary>
	/// <returns> The width of the grid </returns>
	UFUNCTION(BlueprintCallable, Category = "Grid")
	int32 GetWidth() const { return GridWidth; }

	/// <summary>
	/// Returns the height of the grid
	/// </summary>
	/// <returns> The height of the grid </returns>
	UFUNCTION(BlueprintCallable, Category = "Grid")
	int32 GetHeight() const { return GridHeight; }

	/// <summary>
	/// Displays the given attack object on the grid
	/// </summary>
	/// <param name="Attack"></param>
	UFUNCTION(BlueprintCallable, Category = "Attack")
	void ExecuteAttack(UAttack* Attack);
private:
	UPROPERTY()
	TMap<FIntPoint, ABattleTile*> TileGrid;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridWidth = 6;

	UPROPERTY(EditAnywhere, Category = "Grid")
	int32 GridHeight = 3;

	UPROPERTY(EditAnywhere, Category = "Grid")
	float TileSize = 150.0f; // distance between tiles

	/// <summary>
	/// Returns the tile at the given x, y coordinates
	/// </summary>
	/// <param name="Pos"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Grid")
	ABattleTile* GetTileAt(FGridPosition Pos);

	UPROPERTY(EditAnywhere, Category = "Grid")
	TSubclassOf<ABattleTile> TileClass;

	/// <summary>
	/// Changes specific tile to the given state after a specified wait time
	/// </summary>
	/// <param name="X"></param>
	/// <param name="Y"></param>
	/// <param name="WaitTime"></param>
	/// <param name="State"></param>
	/// /// <param name="Damage"></param>
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void AttackTile(FGridPosition Pos, double WaitTime, ETileState State, int32 Damage, bool bParryable, FAttackEffect Effect);
};
