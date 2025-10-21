// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/BattleTile.h"

// Sets default values
ABattleTile::ABattleTile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    CurrentState = ETileState::Default;
    Damage = 0;
}

// Called when the game starts or when spawned
void ABattleTile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABattleTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABattleTile::AffectTile(ETileState NewState, int32 NewDamage)
{
    CurrentState = NewState;
    Damage = NewDamage;
    FVector TilePos = GetActorLocation() + FVector(0,0,10.0f);
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    if (TileMod)
    {
        GetWorld()->DestroyActor(TileMod);
        TileMod = nullptr;
    }

    if (NewState == ETileState::Warning)
    {
        TileMod = GetWorld()->SpawnActor<AActor>(WarningClass,
            TilePos,
            FRotator::ZeroRotator,
            SpawnParams
        );
    }
    else if (NewState == ETileState::Damage)
    {
        TileMod = GetWorld()->SpawnActor<AActor>(DamageClass,
            TilePos,
            FRotator::ZeroRotator,
            SpawnParams
        );
    }
}
