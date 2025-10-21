// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/BattleGrid.h"
#include "Combat/BattleTile.h"
#include "Combat/Attack.h"

// Sets default values
ABattleGrid::ABattleGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABattleGrid::BeginPlay()
{
    Super::BeginPlay();

    FVector Origin = GetActorLocation();

    // Calculate offset so grid is centered on actor
    float TotalWidth = (GridWidth - 1) * TileSize;
    float TotalHeight = (GridHeight - 1) * TileSize;
    FVector GridOffset = FVector(-TotalWidth / 2.0f, -TotalHeight / 2.0f, 0.0f);

    for (int32 y = 0; y < GridHeight; y++)
    {
        for (int32 x = 0; x < GridWidth; x++)
        {
            FVector TilePos = Origin + GridOffset + FVector(x * TileSize, y * TileSize, 0.0f);

            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;

            ABattleTile* Tile = GetWorld()->SpawnActor<ABattleTile>(
                TileClass,
                TilePos,
                FRotator::ZeroRotator,
                SpawnParams
            );

            if (Tile)
            {
                TileGrid.Add(FIntPoint(x,y), Tile);
            }
        }
    }
}

// Called every frame
void ABattleGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

ABattleTile* ABattleGrid::GetTileAt(FGridPosition Pos)
{
    if (ABattleTile** FoundTile = TileGrid.Find(FIntPoint(Pos.x, Pos.y)))
    {
        return *FoundTile;
    }
    return nullptr;
}

void ABattleGrid::AttackTile(FGridPosition Pos, double WaitTime, ETileState State, int32 Damage)
{
    if (Pos.x < 0 || Pos.x >= GridWidth || Pos.y < 0 || Pos.y >= GridHeight)
    {
        return;
    }

    ABattleTile* Tile = GetTileAt(Pos);
    FTimerHandle TimerHandle;
    FTimerDelegate TileDel;
    TileDel.BindUFunction(Tile, FName("AffectTile"), State, Damage);
    if (WaitTime != 0)
    {
        GetWorldTimerManager().SetTimer(TimerHandle, TileDel, WaitTime, false);
    }
    else
    {
        Tile->AffectTile(State, Damage);
    }
}

void ABattleGrid::ExecuteAttack(UAttack* Attack)
{
    if (!IsValid(Attack))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid UAttack pointer %p passed to CombatGrid"), Attack);
        return;
    }

    double CurrentTime = 0;
    // loop through the entire attack structure
    for (FAttackStage currentFrame: Attack->AttackStages)
    {
        CurrentTime += currentFrame.Delay;
        double WarningTime = CurrentTime - currentFrame.WarningLength;

        if (WarningTime <= 0) // If there is no delay for starting attack, but there is a warning
        {
            CurrentTime += -WarningTime;
            WarningTime = 0;
        }

        //loop through each tile in the current frame
        for (FGridPosition Pos : currentFrame.Targets)
        {
            AttackTile(Pos, WarningTime, ETileState::Warning, 0); //start warning
            AttackTile(Pos, CurrentTime, ETileState::Damage, currentFrame.Damage); //start damaging
            AttackTile(Pos, CurrentTime + currentFrame.DamageLength, ETileState::Default, 0); //change tile back to normal
        }
    }
}

int32 ABattleGrid::DamageAtTile(FGridPosition Pos)
{
    ABattleTile* Tile = GetTileAt(Pos);
    return Tile->Damage;
}

FVector ABattleGrid::GetTilePos(FGridPosition Pos)
{
    ABattleTile* Tile = GetTileAt(Pos);
    return Tile->GetActorLocation();
}
