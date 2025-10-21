// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatPlayer.h"

// Sets default values
ACombatPlayer::ACombatPlayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//default values
	StartingPosition = FGridPosition();
	StartingPosition.x = 0;
	StartingPosition.y = 0;
	StartHealth = 10;
	TimeSinceAttack = 10.0f;
	AttackCooldown = 0;
	AttackAllowed = true;
	Focus = EFocus::Default;
}

void ACombatPlayer::Initialize(int32 StartingX, int32 StartingY, int32 StartingHealth, ABattleGrid* BattleGrid){
	StartingPosition = FGridPosition();
	StartingPosition.x = StartingX;
	StartingPosition.y = StartingY;
	StartHealth = StartingHealth;
	Grid = BattleGrid;
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	FVector GridLocation = Grid->GetTilePos(StartingPosition);
	Pawn = GetWorld()->SpawnActor<ACombatPawn>(
		PawnClass,
		GridLocation,
		FRotator::ZeroRotator
	);

    //initialize
    Pawn->Initialize(StartingPosition.x, StartingPosition.y, true, StartHealth, Grid, InvTime);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

int32 ACombatPlayer::GetHealth(){
    return Pawn->GetHealth();
}

void ACombatPlayer::SetMovementAllowed(bool MovementAllowed){
	IsMovementAllowed = MovementAllowed;

	if (IsMovementAllowed && Focus == EFocus::Heal)
	{
		Pawn->EditHealth(HealBuff);
	}
}

// Called when the game starts or when spawned
void ACombatPlayer::BeginPlay()
{
	Super::BeginPlay();
	
    IsMovementAllowed = true;
}

// Called every frame
void ACombatPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TimeSinceAttack += DeltaTime;
	if (TimeSinceAttack >= AttackCooldown) {
		AttackAllowed = true;
	}
}

TArray<EPlayerAttacks> ACombatPlayer::GetAvailablePlayerAttacks(){
	return AvailablePlayerAttacks;
}

void ACombatPlayer::AddAvailablePlayerAttack(EPlayerAttacks Attack){
	AvailablePlayerAttacks.Add(Attack);
}

void ACombatPlayer::AttackGrid(EPlayerAttacks Attack)
{
	if (!AttackAllowed)
	{
		return;
	}
	if(Attack == EPlayerAttacks::NoAttack)
	{
		return;
	}
	if(AttackMapping.Contains(Attack) == false)
	{
		return;
	}

	TSubclassOf<UAttack> AttackClass = *AttackMapping.Find(Attack);

	UAttack* AttackInstance = NewObject<UAttack>(this, AttackClass->GetFName(), RF_NoFlags, AttackClass.GetDefaultObject());
	if(!AttackInstance)
	{
		return;
	}

	if (AttackInstance->bDynamic)
	{
		AttackInstance = AttackInstance->AsStaticAttack(Pawn->GetPosition().x, Pawn->GetPosition().y);
	}
	
	if (Focus == EFocus::Attack)
	{
		AttackInstance->Buff(DamageBuff);
	}

	if (Pawn->ParryBoost)
	{
		AttackInstance->Buff(ParryDamageBuff);
		Pawn->ParryBoost = false;
	}

	Grid->ExecuteAttack(AttackInstance);

	AttackCooldown = AttackInstance->Cooldown;
	TimeSinceAttack = 0.0f;
	AttackAllowed = false;
	Pawn->Stun(AttackInstance->UseTime);
}

void ACombatPlayer::ChangeLeftClickAttack(EPlayerAttacks NewAttack){
	LeftClickAttack = NewAttack;
}

void ACombatPlayer::ChangeRightClickAttack(EPlayerAttacks NewAttack){
	RightClickAttack = NewAttack;
}

void ACombatPlayer::SetBuff(EFocus Foc)
{
	Focus = Foc;
	switch (Foc)
	{
		case EFocus::Attack:
		{
			break;
		}
		case EFocus::Defend:
		{
			Pawn->SetDefend(DefenseBuff);
			break;
		}
		case EFocus::Heal:
		{
			//Pawn->EditHealth(HealBuff); Moved to SetMovementAllowed
			break;
		}
		case EFocus::Default:
		{
			Pawn->SetDefend(0);
			break;
		}
	}
}

void ACombatPlayer::Parry() 
{
	Pawn->AttemptParry();
}
