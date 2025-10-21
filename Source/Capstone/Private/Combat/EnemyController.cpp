// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/EnemyController.h"
#include "Math/UnrealMathUtility.h"
#include "Combat/Attack.h" 
#include "TimerManager.h"

// Sets default values
AEnemyController::AEnemyController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultLocation = FGridPosition();
	DefaultLocation.x = 3;
	DefaultLocation.y = 0;
	DefaultHealth = 50;
}

// Called when the game starts or when spawned



void AEnemyController::Initialize(int32 X, int32 Y, int32 DefHealth, ABattleGrid* Grid)
{
	DefaultLocation.x = X;
	DefaultLocation.y = Y;
	DefaultHealth = DefHealth;
	WorldGrid = Grid;
	FVector CurrentLocation = WorldGrid->GetTilePos(DefaultLocation);
	EnemyPawn = GetWorld()->SpawnActor<ACombatPawn>(
		PawnClass,
		CurrentLocation,
		FRotator::ZeroRotator
	);

	EnemyPawn->Initialize(DefaultLocation.x, DefaultLocation.y, false, DefaultHealth, WorldGrid, InvTime);
}
void AEnemyController::BeginPlay()
{
	Super::BeginPlay();
}
void AEnemyController::OnAttackFinished()
{
	HasFinishedAttack = true;
}
float AEnemyController::BeginEnemyAttack()
{
	if (AttackClasses.Num() == 0)
	{
		return 0;
	}


	HasFinishedAttack = false;

	UAttack* Combo = NewObject<UAttack>(this);

	if (!Combo)
	{
		HasFinishedAttack = true;
		return 0;
	}
	double Len = 0.0;
	int32 Parts = 0;

	while (Parts < MaxSequenceParts && Len < MaxSequenceLength)
	{

		const int32 AttackNumber = FMath::RandRange(0, AttackClasses.Num() - 1);
		TSubclassOf<UAttack> ChooseAttack = AttackClasses[AttackNumber];
		if (!ChooseAttack)
		{
			break;
		}
		UAttack* Attack = NewObject<UAttack>(this, ChooseAttack->GetFName(), RF_NoFlags, ChooseAttack.GetDefaultObject());
		if (!Attack)
		{
			break;
		}
		Combo->Append(Attack);

		const double NewLen = Combo->Length();
		Parts++;

		if (NewLen <= Len)
		{
			break;
		}

		Len = NewLen;

	}

	if (Parts == 0)
	{
		HasFinishedAttack = true;
		return 0;
	}
		
	WorldGrid->ExecuteAttack(Combo);

	double SeqLength = Combo->Length();
	if (SeqLength <= 0.0)
	{
		SeqLength = 0.1;
	}

	GetWorldTimerManager().SetTimer(AttackFinishTimer, this, &AEnemyController::OnAttackFinished, SeqLength, false);

	return SeqLength;
}

 
void AEnemyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	static float TimeSinceLastMove = 0.0f;
	TimeSinceLastMove += DeltaTime;
	if (TimeSinceLastMove >= 2.0f) // move once per second
	{
		MoveRandomOnGrid();
		TimeSinceLastMove = 0.0f;
	}

}


void AEnemyController::MoveRandomOnGrid()
{
	// Randomly add or subtract 1 to X and Y
	int32 RandX = FMath::RandRange(-1, 1);
	int32 RandY = FMath::RandRange(-1, 1);

	// Update grid location
	FVector2D NewLocation(RandX, RandY);
	EnemyPawn->Move(NewLocation);

}

int32 AEnemyController::GetHealth() {
	return EnemyPawn->GetHealth();
}