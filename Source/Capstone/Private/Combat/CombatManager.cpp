// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/CombatManager.h"
#include "LevelTransitionHandler.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ACombatManager::ACombatManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bEnablePreCombatCountdown = true;
	PreCombatCountdownSeconds = 3.0f;
	Difficulty = EDifficulty::Normal;
}

// Called when the game starts or when spawned
void ACombatManager::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACombatManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//AssessWLState();
}

void ACombatManager::AssessWLState()
{
	//If contextually appropriate show feedback
	if (PlayerC->GetHealth() <= 0)
	{
		PlayerLoses();
	}
	if (EnemyC->GetHealth() <= 0)
	{
		PlayerWins();
	}
	//otherwise just returning is fine
}

void ACombatManager::BeginTurnCycle()
{
	//Summon menu and store struct with player's choices (or menu will tell the player controller what was picked?)
	//Unsummon menu
	//Tell enemy to begin it's attack
		//TODO: implement call to enemy controller's select attack
}

void ACombatManager::PlayerWins()
{
	UE_LOG(LogTemp, Display, TEXT("Player wins with %d health remaining"), PlayerC->GetHealth());
	TransitionToOverworld();
}

void ACombatManager::PlayerLoses()
{
	UE_LOG(LogTemp, Display, TEXT("Player Lost :( Enemy has %d health remaining"), 5);
	TransitionToOverworld();
}

void ACombatManager::TransitionToOverworld()
{
	if (ULevelTransitionHandler* TransitionHandler = Cast<ULevelTransitionHandler>(GetGameInstance()))
	{
		TransitionHandler->ReturnToOverworld();
	}
}

void ACombatManager::AssignControllers()
{
	//Initialize Grid
	Grid = GetWorld()->SpawnActor<ABattleGrid>(
		GridClass,
		FVector(0, 0, 0),
		FRotator::ZeroRotator
	);

	//Create spawning parameters for pawns
	FActorSpawnParameters Params = FActorSpawnParameters();
	Params.bNoFail = true;
	Params.Owner = this;

	//Spawn Enemy Controller
	EnemyC = GetWorld()->SpawnActor<ACombatEnemy>(
		EnemyClass,
		FVector(0, 0, 0),
		FRotator::ZeroRotator,
		Params
	);

	//Initialize Enemy Controller
	//FVector GridLocationE = Grid->GetTilePos(FGridPosition(4, 1));
	EnemyC->Initialize(4, 1, Grid);

	//Spawn Player Controller
	PlayerC = GetWorld()->SpawnActor<ACombatPlayer>(
		PlayerClass,
		FVector(0, 0, 0),
		FRotator::ZeroRotator,
		Params
	);

	//Initialize Player Controller
	//FVector GridLocationP = Grid->GetTilePos(FGridPosition(1,1));
	PlayerC->Initialize(1, 1, Grid);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->Possess(PlayerC);

	// --- Pre-combat countdown entry point ---
	if (bEnablePreCombatCountdown && PreCombatCountdownSeconds > 0.0f)
	{
		StartPreCombatCountdown();
	}
	else
	{
		BeginTurnCycle();
	}
}

void ACombatManager::StartPreCombatCountdown()
{
	LockPlayerControl(true);

	CountdownRemaining = PreCombatCountdownSeconds;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&ACombatManager::OnCountdownTick,
			1.0f,
			true
		);
	}
}

void ACombatManager::OnCountdownTick()
{
	CountdownRemaining -= 1.0f;

	int32 SecondsLeft = FMath::CeilToInt(CountdownRemaining);

	if (SecondsLeft > 0)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}

	LockPlayerControl(false);
	BeginTurnCycle();
}

void ACombatManager::LockPlayerControl(bool bLock)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->SetIgnoreMoveInput(bLock);
		PC->SetIgnoreLookInput(bLock);
	}
}

void ACombatManager::ChangeDifficulty(int diff)
{
	switch (diff)
	{
		case 0:
			//easy mode
			Difficulty = EDifficulty::Easy;
			Grid->Difficulty = EDifficulty::Easy;
			break;
		case 2:
			//hard mode
			Difficulty = EDifficulty::Hard;
			Grid->Difficulty = EDifficulty::Hard;
			break;
		default:
			//normal mode (1)
			Difficulty = EDifficulty::Normal;
			Grid->Difficulty = EDifficulty::Normal;
			break;
	}
}
