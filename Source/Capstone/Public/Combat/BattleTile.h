// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Attack.h"

#include "BattleTile.generated.h"

// Enum for tile states
UENUM(BlueprintType)
enum class ETileState : uint8
{
	Default UMETA(DisplayName = "Default"),
	Warning UMETA(DisplayName = "Warning"),
	Damage  UMETA(DisplayName = "Damage"),
	Unparriable UMETA(DisplayName="Unparryable")
};

class UNiagaraComponent;
class UNiagaraSystem;

/// <summary>
/// Class used to represent one of the tiles on battle grid, can have states default, damage, and warning
/// </summary>
UCLASS()
class CAPSTONE_API ABattleTile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABattleTile();

	UPROPERTY()
	UNiagaraComponent* ActiveEffectComponent = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Classes (set in the editor)
	UPROPERTY(EditAnywhere, Category = "States")
	TSubclassOf<AActor> WarningClass;

	UPROPERTY(EditAnywhere, Category = "States")
	TSubclassOf<AActor> DamageClass;

	UPROPERTY(EditAnywhere, Category = "States")
	TSubclassOf<AActor> UnparriableClass;

	UPROPERTY()
	AActor* TileMod = nullptr;

	// Current state of the tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	ETileState CurrentState;

	UPROPERTY()
	int32 Damage;

	UPROPERTY() bool bParriable = true;

	/// <summary>
	/// Given a ETile State, changes the tile to reflect that state
	/// </summary>
	/// <param name="NewState"></param>
	/// /// <param name="Damage"></param>
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void AffectTile(ETileState NewState, int32 NewDamage,bool bNewParryable, FAttackEffect Effect);
};
