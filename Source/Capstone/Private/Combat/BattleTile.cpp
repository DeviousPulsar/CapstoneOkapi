// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/BattleTile.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

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

void ABattleTile::AffectTile(ETileState NewState, int32 NewDamage, bool bNewParryable, FAttackEffect Effect)
{
    CurrentState = NewState;
    Damage = NewDamage;
    bParriable = bNewParryable;
    FVector TilePos = GetActorLocation() + FVector(0,0,10.0f);
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    if (TileMod)
    {
        GetWorld()->DestroyActor(TileMod);
        TileMod = nullptr;
    }

    if (ActiveEffectComponent){
        // using autodelete, uncomment if you want to switch to manually deleting


        // ActiveEffectComponent->Deactivate();
        // ActiveEffectComponent->DestroyComponent();
        // ActiveEffectComponent = nullptr;
    }

    TSubclassOf<AActor> ActorToSpawn = nullptr;
    switch(NewState){
        case ETileState::Warning:
            ActorToSpawn = WarningClass;
            break;
        case ETileState::Damage:
            ActorToSpawn = DamageClass;
            break;
        case ETileState::Unparriable:
            ActorToSpawn = UnparriableClass;
            break;
        default:
            break;
    }

    if(Effect.Effect){
        ActiveEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                Effect.Effect,
                TilePos,
                FRotator::ZeroRotator,
                FVector(1.0f),
                true, // autoDestroy
                true  // autoActivate
                );

        ActiveEffectComponent->SetVariableFloat(TEXT("AttackDuration"), Effect.Duration);
        ActiveEffectComponent->SetVariableFloat(TEXT("EffectScale"), Effect.Scale);
    }
    else if(ActorToSpawn){
        TileMod = GetWorld()->SpawnActor<AActor>(ActorToSpawn,
            TilePos,
            FRotator::ZeroRotator,
            SpawnParams
        );
    }

    
}
