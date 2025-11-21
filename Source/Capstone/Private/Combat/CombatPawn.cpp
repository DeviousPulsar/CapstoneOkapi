// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatPawn.h"
#include "Combat/GridPosition.h"
#include "Combat/PlayerInputController.h"
#include "Combat/Attack.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"

// Sets default values
ACombatPawn::ACombatPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//default values to fill state variables, may be overwritten with Initialize
	CurrentPosition = FGridPosition();
	CurrentPosition.x = 0;
	CurrentPosition.y = 0;
	IsPlayer = true;
	InitialHealth = 100;
	PawnHealth = 100;
	Grid = nullptr;
	MoveAllowed = true;
	TimeSinceStun = 0.0f;
	TimeSinceVulnerable = 0.0f;
	TimeSinceParry = 0.0f;
	Vulnerable = true;
	Defend = 1.0;
	Parry = false;
	ParryBoost = false;
}

bool ACombatPawn::GetIsPlayer() {
	return IsPlayer;
}

void ACombatPawn::Initialize(int X, int Y, ABattleGrid* BattleGrid) {
	this->CurrentPosition = FGridPosition(X, Y);
	this->Grid = BattleGrid;
	this->IsPlayer = X < BattleGrid->GetWidth() / 2;
	this->PawnHealth = InitialHealth;

	//move pawn to location on grid
	if (Grid) {
		FVector GridWorldLocation = Grid->GetTilePos(CurrentPosition);
		SetActorLocation(GridWorldLocation, false);
		StartPosition = GridWorldLocation;
		EndPosition = GridWorldLocation;
	}
}

/// @brief Used to move the pawn on the board. Enforces the rule about the enemy not being allowed on the
/// player side and vice versa. Also disallows movement off the board.
/// @param Value 
void ACombatPawn::Move(FVector2D Vector) {
	//disallow move if grid is not valid
	if (Grid == nullptr) {
		return;
	}

	//move disallowed, do nothing
	if (MoveAllowed == false || bIsFrozen) {
		return;
	}

	ForceMove(Vector);
}

void ACombatPawn::ForceMove(FVector2D Vector)
{
	//get x and y movement out of input
	int X = FMath::RoundToInt(Vector.X);
	int Y = FMath::RoundToInt(Vector.Y);

	int32 IsPlayerOffset;
	if (IsPlayer) {
		IsPlayerOffset = 0;
	}
	else {
		IsPlayerOffset = Grid->GetWidth() / 2;
	}

	bool bMoved = false;
	int UsedX = 0;
	int UsedY = 0;

	//do movement
	if (X != 0) {
		int32 RequestedXPosition = CurrentPosition.x + X;
		if (RequestedXPosition >= 0 + IsPlayerOffset && RequestedXPosition < Grid->GetWidth() / 2 + IsPlayerOffset) {
			//allow move in x direction
			CurrentPosition.x = RequestedXPosition;
			bMoved = true;
			UsedX = X;
		}
	}
	if (Y != 0) {
		int32 RequestedYPosition = CurrentPosition.y + Y;
		if (RequestedYPosition >= 0 && RequestedYPosition < Grid->GetHeight()) {
			//allow move in y direction
			CurrentPosition.y = RequestedYPosition;
			bMoved = true;
			UsedY = Y;
		}
	}
	//move pawn on grid
	FVector GridWorldLocation = Grid->GetTilePos(CurrentPosition);
	EndPosition = GridWorldLocation;
	//SetActorLocation(GridWorldLocation, false);
	Stun(MoveStun);

	if (bMoved)
	{
		if (UsedX > 0 && MoveForwardMontage)
		{
			PlayMoveMontage(MoveForwardMontage);
		}
		else if (UsedX < 0 && MoveBackwardMontage)
		{
			PlayMoveMontage(MoveBackwardMontage);
		}
		else if (UsedY < 0 && MoveLeftMontage)
		{
			PlayMoveMontage(MoveLeftMontage);
		}
		else if (UsedY > 0 && MoveRightMontage)
		{
			PlayMoveMontage(MoveRightMontage);
		}
	}
}

FGridPosition ACombatPawn::GetPosition() {
	return CurrentPosition;
}

int32 ACombatPawn::GetHealth() {
	return PawnHealth;
}

int32 ACombatPawn::EditHealth(int32 AmountToChange) {
	PawnHealth += AmountToChange;
	PawnHealth = FMath::Clamp(PawnHealth, 0, InitialHealth);

	if (AmountToChange > 0) {
		//play heal vfx
		ActivateEffect(HealingComponent);
	}

	return PawnHealth;
}

// Called when the game starts or when spawned
void ACombatPawn::BeginPlay()
{
	Super::BeginPlay();

	//Spawn effects
	//UNiagaraSystem*, Duration, Scale
	//NOTE: duration is ignored for now, may not be needed for these
	HealingComponent = SpawnEffect(HealEffect, 4, 1);
	InvulnerableComponent = SpawnEffect(InvulnerableEffect, InvTime, 1);
	ParryComponent = SpawnEffect(ParryEffect, 1, 1);

	DeactivateEffect(HealingComponent);
	DeactivateEffect(InvulnerableComponent);
	DeactivateEffect(ParryComponent);
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

			if (Parry && bHitParryable)
			{
				ParryBoost = true;
				Parry = false;
				ParryProt = true;
				DeactivateEffect(ParryComponent);
			}
			else
			{
				ParryBoost = false;
				EditHealth(-Defend * Damage);
				PlayHitReactMontage();
			}
			Vulnerable = false;
			TimeSinceVulnerable = 0.0f;

			//play invulnerable effect
			ActivateEffect(InvulnerableComponent);
		}
	}

	TimeSinceStun += DeltaTime;
	TimeSinceParry += DeltaTime;

	if (TimeSinceStun >= MoveCooldown)
	{
		MoveAllowed = true;
		StartPosition = Grid->GetTilePos(CurrentPosition);
		SetActorLocation(StartPosition);
	}
	else
	{
		float ResultX = FMath::InterpEaseInOut<float>(StartPosition.X, EndPosition.X, TimeSinceStun / MoveCooldown, 2.0f);
		float ResultY = FMath::InterpEaseInOut<float>(StartPosition.Y, EndPosition.Y, TimeSinceStun / MoveCooldown, 2.0f);
		SetActorLocation(FVector(ResultX, ResultY, EndPosition.Z), false);
	}

	if (TimeSinceParry >= ParryWindow)
	{
		Parry = false;
		DeactivateEffect(ParryComponent);
	}

	if (!Vulnerable)
	{
		TimeSinceVulnerable += DeltaTime;
		//if we are invulnerable from taking damage use this countdown
		if (!ParryProt && (TimeSinceVulnerable >= InvTime))
		{
			Vulnerable = true;
			DeactivateEffect(InvulnerableComponent);
		}
		//if we are invulnerable from a successful parry use this countdown
		if (ParryProt && (TimeSinceVulnerable >= ParryProtTime))
		{
			Vulnerable = true;
			ParryProt = false;
			DeactivateEffect(InvulnerableComponent);
		}
	}
}

void ACombatPawn::Stun(float Length)
{
	MoveAllowed = false;
	TimeSinceStun = 0.0f;
	MoveCooldown = Length;
}

void ACombatPawn::SetDefend(float DamageBlocked)
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
		//parry effect
		ActivateEffect(ParryComponent);
		PlayParryMontage();
	}
}

void ACombatPawn::SetMovementAllowed(bool MovementAllowed)
{
	bIsFrozen = !MovementAllowed;
	if (MovementAllowed) {
		DeactivateEffect(HealingComponent);
	}
}

void ACombatPawn::PlayAttackMontage(FName Section)
{
	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh) return;
	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim || !AttackMontage) return;
	Anim->Montage_Play(AttackMontage);
	if (Section != NAME_None)
	{
		Anim->Montage_JumpToSection(Section, AttackMontage);
	}
}

void ACombatPawn::PlayHitReactMontage(FName Section)
{
	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh) return;
	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim || !HitReactMontage) return;
	Anim->StopAllMontages(0.1f);
	Anim->Montage_Play(HitReactMontage);
	if (Section != NAME_None)
	{
		Anim->Montage_JumpToSection(Section, HitReactMontage);
	}
}

void ACombatPawn::PlayMoveMontage(UAnimMontage* Montage, FName Section)
{
	if (!Montage)
	{
		return;
	}
	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh) return;
	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim) return;
	Anim->Montage_Play(Montage);
	if (Section != NAME_None)
	{
		Anim->Montage_JumpToSection(Section, Montage);
	}
}

void ACombatPawn::PlayParryMontage(FName Section)
{
	if (!ParryMontage)
	{
		return;
	}
	USkeletalMeshComponent* Mesh = FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh) return;
	UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (!Anim) return;
	Anim->Montage_Play(ParryMontage);
	if (Section != NAME_None)
	{
		Anim->Montage_JumpToSection(Section, ParryMontage);
	}
}

void ACombatPawn::ReturnToCenter() {
	FGridPosition Center = FGridPosition(1 + (IsPlayer ? 0 : Grid->GetWidth() / 2), 1);
	ForceMove(FVector2D(Center.x - CurrentPosition.x, Center.y - CurrentPosition.y));
}

UNiagaraComponent* ACombatPawn::SpawnEffect(UNiagaraSystem* Effect, double Duration, double Scale) {
	if (Effect) {
		UNiagaraComponent* ActiveEffectComponent =
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				Effect,
				GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				false, //autodestroy
				false //autoactivate
			);
		//Ignore duration for now
		//ActiveEffectComponent->SetVariableFloat(TEXT("AttackDuration"), Duration);
		ActiveEffectComponent->SetVariableFloat(TEXT("EffectScale"), Scale);
		return ActiveEffectComponent;
	}
	return nullptr;
}

void ACombatPawn::ActivateEffect(UNiagaraComponent* EffectComponent) {
	if (EffectComponent) {
		EffectComponent->ResetSystem();
		EffectComponent->Activate(true);
		//EffectComponent->SetVisibility(true);
	}

}

void ACombatPawn::DeactivateEffect(UNiagaraComponent* EffectComponent) {
	if (EffectComponent) {
		EffectComponent->Deactivate();
		//EffectComponent->SetVisibility(false);
	}

}
