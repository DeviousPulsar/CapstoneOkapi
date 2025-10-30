// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/CombatManager.h"
#include "LevelTransitionHandler.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACombatManager::ACombatManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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
	AssessWLState();
}

UFUNCTION(BlueprintCallable) void ACombatManager::AssessWLState()
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

UFUNCTION(BlueprintCallable) void ACombatManager::BeginTurnCycle()
{
	//Summon menu and store struct with player's choices (or menu will tell the player controller what was picked?)
	//Unsummon menu
	//Tell enemy to begin it's attack
		//TODO: implement call to enemy controller's select attack
}

UFUNCTION(BlueprintCallable) void ACombatManager::PlayerWins()
{
	UE_LOG(LogTemp, Display, TEXT("Player wins with %d health remaining"), PlayerC->GetHealth());
	TransitionToOverworld();
}

UFUNCTION(BlueprintCallable) void ACombatManager::PlayerLoses()
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

UFUNCTION(BlueprintCallable) void ACombatManager::AssignControllers()
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
	EnemyC = GetWorld()->SpawnActor<AEnemyController>(
		EnemyClass,
		FVector(0, 0, 0),
		FRotator::ZeroRotator,
		Params
	);

	//Initialize Enemy Controller
	//FVector GridLocationE = Grid->GetTilePos(FGridPosition(4, 1));
	EnemyC->Initialize(4, 1, 100, Grid);


	//Spawn Player Controller
	PlayerC = GetWorld()->SpawnActor<ACombatPlayer>(
		PlayerClass,
		FVector(0, 0, 0),
		FRotator::ZeroRotator, 
		Params
	);

	//Initialize Player Controller
	//FVector GridLocationP = Grid->GetTilePos(FGridPosition(1,1));
	PlayerC->Initialize(1, 1, 70, Grid);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->Possess(PlayerC);
}