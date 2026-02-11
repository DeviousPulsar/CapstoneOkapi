// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/BattleGrid.h"
#include "Combat/BattleTile.h"
#include "Combat/Attack.h"

#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"

// Sets default values
ABattleGrid::ABattleGrid()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    PlayerPreviewZOffset = 2.0f;
    PlayerPreviewDecalSize = FVector(64.f, 64.f, 64.f);
}

// Called when the game starts or when spawned
void ABattleGrid::BeginPlay()
{
    Super::BeginPlay();

    FVector Origin = GetActorLocation();

    TArray<ABattleTile*> TilesInLevel;
    TArray<AActor*> TileActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattleTile::StaticClass(), TileActors);

    float TileNum = 0;
    for (AActor* tile : TileActors)
    {
        TilesInLevel.Add(Cast<ABattleTile>(tile));
        TileNum++;
    }

    for (ABattleTile* CurrentTile : TilesInLevel)
    {
        TileGrid.Add(FIntPoint(CurrentTile->XPos, CurrentTile->YPos), CurrentTile);
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

void ABattleGrid::AttackTile(FGridPosition Pos, double WaitTime, ETileState State, int32 Damage, bool bParriable, FAttackEffect Effect)
{
    if (Pos.x < 0 || Pos.x >= GridWidth || Pos.y < 0 || Pos.y >= GridHeight)
    {
        return;
    }

    ABattleTile* Tile = GetTileAt(Pos);
    if (!IsValid(Tile))
    {
        return;
    }

    FTimerHandle TimerHandle;
    FTimerDelegate TileDel;
    TileDel.BindUFunction(Tile, FName("AffectTile"), State, Damage, bParriable, Effect);
    if (WaitTime != 0)
    {
        GetWorldTimerManager().SetTimer(TimerHandle, TileDel, WaitTime, false);
    }
    else
    {
        Tile->AffectTile(State, Damage, bParriable, Effect);
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
    for (FAttackStage currentFrame : Attack->AttackStages)
    {
        CurrentTime += currentFrame.Delay;
        double WarningTime = CurrentTime - currentFrame.WarningLength;

        if (WarningTime <= 0)
        {
            CurrentTime += -WarningTime;
            WarningTime = 0;
        }

        for (FGridPosition Pos : currentFrame.Targets)
        {
            ETileState StateToUse;

            UNiagaraSystem* WarningVFX = nullptr;
            UNiagaraSystem* AttackVFX = nullptr;

            if (currentFrame.bParriable)
            {
                StateToUse = ETileState::Damage;

                if (Attack->WarningEffect) {
                    WarningVFX = Attack->WarningEffect;
                }
                if (Attack->AttackEffect) {
                    AttackVFX = Attack->AttackEffect;
                }
            }
            else
            {
                StateToUse = ETileState::Unparriable;

                if (Attack->UnparryableWarningEffect) {
                    WarningVFX = Attack->UnparryableWarningEffect;
                }
                if (Attack->UnparryableAttackEffect) {
                    AttackVFX = Attack->UnparryableAttackEffect;
                }
            }

            if (currentFrame.TargetWarningEffect) {
                WarningVFX = currentFrame.TargetWarningEffect;
            }
            if (currentFrame.TargetAttackEffect) {
                AttackVFX = currentFrame.TargetAttackEffect;
            }

            FAttackEffect WarningEffect = FAttackEffect(WarningVFX, currentFrame.WarningLength, 1);
            FAttackEffect AttackingEffect = FAttackEffect(AttackVFX, currentFrame.DamageLength, 1);
            FAttackEffect NoEffect = FAttackEffect(nullptr, 0, 0);

            if (currentFrame.WarningLength > 0)
            {
                AttackTile(Pos, WarningTime, ETileState::Warning, 0, currentFrame.bParriable, WarningEffect);
            }
            if (currentFrame.DamageLength > 0)
            {
                AttackTile(Pos, CurrentTime, StateToUse, currentFrame.Damage, currentFrame.bParriable, AttackingEffect);
            }
            AttackTile(Pos, CurrentTime + currentFrame.DamageLength, ETileState::Default, 0, true, NoEffect);
        }
    }
}

int32 ABattleGrid::DamageAtTile(FGridPosition Pos)
{
    ABattleTile* Tile = GetTileAt(Pos);
    if (!IsValid(Tile))
    {
        return 0;
    }
    return Tile->Damage;
}

FVector ABattleGrid::GetTilePos(FGridPosition Pos)
{
    ABattleTile* Tile = GetTileAt(Pos);
    if (!IsValid(Tile))
    {
        return FVector::ZeroVector;
    }
    return Tile->GetActorLocation();
}

bool ABattleGrid::IsParriableAtTile(FGridPosition Pos)
{
    if (ABattleTile* Tile = GetTileAt(Pos))
    {
        return Tile->bParriable;
    }
    return true;
}

void ABattleGrid::ClearPlayerPreview()
{
    for (UDecalComponent* Decal : PlayerPreviewDecalPool)
    {
        if (IsValid(Decal))
        {
            Decal->SetVisibility(false);
        }
    }
}

void ABattleGrid::ShowPlayerPreview(const TArray<FGridPosition>& Tiles)
{
    if (!PlayerPreviewDecalMaterial)
    {
        return;
    }

    ClearPlayerPreview();

    TArray<FGridPosition> ValidTiles;
    ValidTiles.Reserve(Tiles.Num());

    for (const FGridPosition& P : Tiles)
    {
        if (P.x < 0 || P.x >= GridWidth || P.y < 0 || P.y >= GridHeight)
        {
            continue;
        }

        if (!IsValid(GetTileAt(P)))
        {
            continue;
        }

        ValidTiles.Add(P);
    }

    if (ValidTiles.Num() == 0)
    {
        return;
    }

    while (PlayerPreviewDecalPool.Num() < ValidTiles.Num())
    {
        UDecalComponent* NewDecal = NewObject<UDecalComponent>(this);
        if (!NewDecal)
        {
            break;
        }

        NewDecal->RegisterComponentWithWorld(GetWorld());
        if (GetRootComponent())
        {
            NewDecal->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        }

        NewDecal->SetDecalMaterial(PlayerPreviewDecalMaterial);
        NewDecal->DecalSize = PlayerPreviewDecalSize;
        NewDecal->SetVisibility(false);

        PlayerPreviewDecalPool.Add(NewDecal);
    }

    const int32 Count = FMath::Min(PlayerPreviewDecalPool.Num(), ValidTiles.Num());
    for (int32 i = 0; i < Count; i++)
    {
        UDecalComponent* Decal = PlayerPreviewDecalPool[i];
        if (!IsValid(Decal))
        {
            continue;
        }

        const FVector WorldPos = GetTilePos(ValidTiles[i]) + FVector(0, 0, PlayerPreviewZOffset);
        Decal->SetWorldLocation(WorldPos);
        Decal->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
        Decal->SetVisibility(true);
    }
}
