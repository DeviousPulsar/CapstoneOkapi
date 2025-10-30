// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatPawn.h"
#include "Combat/GridPosition.h"
#include "Combat/PlayerInputController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Engine/Engine.h"

// Sets default values
ACombatPawn::ACombatPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//default values to fill variables, will be overwritten with Initialize
	CurrentPosition = FGridPosition();
	CurrentPosition.x = 0;
	CurrentPosition.y = 0;
	IsPlayer = true;
	PawnHealth = 10;
	Grid = nullptr;
	MoveAllowed = true;
	TimeSinceStun = 0.0f;
	TimeSinceVulnerable = 0.0f;
	TimeSinceParry = 0.0f;
	Vulnerable = true;
	Defend = 0;
	Parry = false;
	ParryBoost = false;
}

bool ACombatPawn::GetIsPlayer(){
	return IsPlayer;
}

void ACombatPawn::Initialize(int X, int Y, bool bIsPlayer, int32 StartingHealth, ABattleGrid* BattleGrid, float InvulTime){
	this->CurrentPosition = FGridPosition(X, Y);
	this->IsPlayer = bIsPlayer;
	this->InitialHealth = StartingHealth;
	this->PawnHealth = StartingHealth;
	this->Grid = BattleGrid;
	this->InvTime = InvulTime;

	//move pawn to location on grid
	if(Grid){
		FVector GridWorldLocation = Grid->GetTilePos(CurrentPosition);
		SetActorLocation(GridWorldLocation, false);
	}
	

}

/// @brief Used to move the pawn on the board. Enforces the rule about the enemy not being allowed on the
/// player side and vice versa. Also disallows movement off the board.
/// @param Value 
void ACombatPawn::Move(FVector2D Vector){
	//disallow move if grid is not valid
	if(Grid == nullptr){
		return; 
	}

	//move disallowed, do nothing
	if (MoveAllowed == false || bIsFrozen){
		return;
	}

	//get x and y movement out of input
	int X = FMath::RoundToInt(Vector.X);
	int Y = FMath::RoundToInt(Vector.Y);

	int32 IsPlayerOffset;
	if(IsPlayer){
		IsPlayerOffset = 0;
	}
	else{
		IsPlayerOffset = 3;
	}

	//do movement
	if(X != 0){
		int32 RequestedXPosition = CurrentPosition.x + X;
		if (RequestedXPosition >= 0 + IsPlayerOffset && RequestedXPosition < 3 + IsPlayerOffset){
			//allow move in x direction
			CurrentPosition.x = RequestedXPosition;
			//move pawn on grid
			FVector GridWorldLocation = Grid->GetTilePos(CurrentPosition);
			SetActorLocation(GridWorldLocation, false);
			Stun(MoveStun);
		}
	}
	if(Y != 0){
		int32 RequestedYPosition = CurrentPosition.y + Y;
		if (RequestedYPosition >= 0 && RequestedYPosition < 3){
			//allow move in y direction
			CurrentPosition.y = RequestedYPosition;
			//move pawn on grid
			FVector GridWorldLocation = Grid->GetTilePos(CurrentPosition);
			SetActorLocation(GridWorldLocation, false);
			Stun(MoveStun);
		}
	}
}

FGridPosition ACombatPawn::GetPosition(){
	return CurrentPosition;
}

int32 ACombatPawn::GetHealth(){
	return PawnHealth;
}

int32 ACombatPawn::EditHealth(int32 AmountToChange){
	PawnHealth += AmountToChange;
	PawnHealth = FMath::Clamp(PawnHealth, 0, InitialHealth);

	return PawnHealth;
}

// Called when the game starts or when spawned
void ACombatPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACombatPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//check for damage to pawn
	if (Grid)
	{
		int32 Damage = Grid->DamageAtTile(CurrentPosition);
		if (Damage != 0 && Vulnerable && !bIsFrozen) 
		{
			bool bHitParryable = Grid->IsParriableAtTile(CurrentPosition);

			//deal damage, make pawn immune to damage, then scedule turing them vulnerable again in specified amount of time

			if (Defend != 0)
			{
				Damage -= Defend;
				if (Damage < 0)
				{
					Damage = 0;
				}
			}
			
			if (Parry && bHitParryable)
			{
				ParryBoost = true;
				Parry = false;
				ParryProt = true;
			}
			else
			{
				ParryBoost = false;
				EditHealth(-Damage);
			}
			Vulnerable = false;
			TimeSinceVulnerable = 0.0f;
		}
	}

	TimeSinceStun += DeltaTime;
	TimeSinceParry += DeltaTime;

	if (TimeSinceStun >= MoveCooldown)
	{
		MoveAllowed = true;
	}

	if (TimeSinceParry >= ParryWindow)
	{
		Parry = false;
	}

	if (!Vulnerable)
	{
		TimeSinceVulnerable += DeltaTime;
		//if we are invulnerable from taking damage use this countdown
		if (!ParryProt && (TimeSinceVulnerable >= InvTime))
		{
			Vulnerable = true;
		}
		//if we are invulnerable from a successful parry use this countdown
		if (ParryProt && (TimeSinceVulnerable >= ParryProtTime))
		{
			Vulnerable = true;
			ParryProt = false; 
		}
	}
}

void ACombatPawn::Stun(float Length)
{
	MoveAllowed = false;
	TimeSinceStun = 0.0f;
	MoveCooldown = Length;
}

void ACombatPawn::SetDefend(int32 DamageBlocked)
{
	Defend = DamageBlocked;
}

void ACombatPawn::AttemptParry()
{
	// Don't allow parries if we are currently invulnerable, or if we are stunned
	if (!MoveAllowed || !Vulnerable || bIsFrozen)
	{
		return;
	}
	if (TimeSinceParry >= ParryCooldown)
	{
		Parry = true;
		TimeSinceParry = 0.0f;
		Stun(ParryStunTime);
	}
}

void ACombatPawn::SetMovementAllowed(bool MovementAllowed)
{
	bIsFrozen = !MovementAllowed;
}
