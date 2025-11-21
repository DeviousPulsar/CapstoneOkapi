// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatEnemy.h"
#include "Math/UnrealMathUtility.h"
#include "Combat/Attack.h"
#include "Combat/AttackSequence.h" 
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ACombatEnemy::ACombatEnemy()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACombatEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void ACombatEnemy::OnAttackFinished()
{
	HasFinishedAttack = true;
}

void ACombatEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	static float TimeSinceLastMove = 0.0f;
	static float TimeSinceLastAttack = 0.0f;
	static float LastAttackUseTime = 0.0f;
	static float LastAttackCooldown = 0.0f;

	if (!HasFinishedAttack) {
		TimeSinceLastMove += DeltaTime;
		TimeSinceLastAttack += DeltaTime;

		if (TimeSinceLastAttack >= LastAttackCooldown && TimeSinceLastMove >= MoveStun)
		{
			if (IsValid(CurrentSequence) && CurrentSequence->HasNext())
			{
				auto val = CurrentSequence->ExecuteNext(Grid);
				LastAttackUseTime = val.UseTime;
				LastAttackCooldown = val.Cooldown;

				TimeSinceLastAttack = 0;
			}
			else
			{
				OnAttackFinished();
			}
		}
		else if (TimeSinceLastMove >= MovementCooldown && TimeSinceLastAttack >= LastAttackUseTime)
		{
			MoveRandomOnGrid();
			TimeSinceLastMove = 0.0f;
		}
	}
}

float ACombatEnemy::BeginEnemyAttack()
{
	if (AttackSequences.Num() <= 0)
	{
		return 0;
	}

	HasFinishedAttack = false;

	CurrentSeqeunceIndex++;
	CurrentSeqeunceIndex %= AttackSequences.Num();
	CurrentSequence = NewObject<UAttackSequence>(this, AttackSequences[CurrentSeqeunceIndex]);
	CurrentSequence->SetUser(this);
	CurrentSequence->InitalizeSequence();

	return CurrentSequence->Length();
}

void ACombatEnemy::MoveRandomOnGrid()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	ACombatPawn* PlayerCombat = Cast<ACombatPawn>(PlayerPawn);

	int PlayerPosY = PlayerCombat->GetPosition().y;
	int EnemyPosY = GetPosition().y;
	// Randomly add or subtract 1 to X 
	int32 RandX = FMath::RandRange(-1, 1);
	int32 RandY = FMath::RandRange(-1, 1);

	int32 dY;
	if (EnemyPosY == PlayerPosY)
	{
		if (EnemyPosY > 1)
		{
			dY = -1;
		}
		if (EnemyPosY < 1)
		{
			dY = 1;
		}
		if (EnemyPosY == 1)
		{
			dY = (FMath::RandBool() ? 1 : -1);
		}
		FVector2D NewLocation(RandX, dY);
	}
	else
	{
		dY = RandY;
	}
	FVector2D NewLocation(RandX, dY);
	Move(NewLocation);
}

// ==================== 下面是新增的动画逻辑函数 ====================

float ACombatEnemy::BeginEnemyAttack_Anim()
{
	float Duration = BeginEnemyAttack();

	bIsAttacking = true;

	float UseDuration = Duration;
	if (AttackAnimDurationOverride > 0.0f)
	{
		UseDuration = AttackAnimDurationOverride;
	}

	if (UseDuration > 0.0f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			AttackAnimTimer,
			this,
			&ACombatEnemy::OnAttackAnimFinished,
			UseDuration,
			false
		);
	}

	return Duration;
}

void ACombatEnemy::OnAttackAnimFinished()
{
	bIsAttacking = false;
}

void ACombatEnemy::MoveRandomOnGrid_Anim()
{
	bIsMoving = true;
	MoveRandomOnGrid();
	bIsMoving = false;
}

void ACombatEnemy::PlayHitReact()
{
	if (bIsParrying)
	{
		return;
	}

	bIsHitReact = true;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			HitReactTimer,
			this,
			&ACombatEnemy::OnHitReactFinished,
			HitReactDuration,
			false
		);
	}
}

void ACombatEnemy::OnHitReactFinished()
{
	bIsHitReact = false;
}

void ACombatEnemy::BeginParry()
{
	bIsParrying = true;
	bIsAttacking = false;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ParryTimer,
			this,
			&ACombatEnemy::OnParryFinished,
			ParryDuration,
			false
		);
	}
}

void ACombatEnemy::OnParryFinished()
{
	bIsParrying = false;
}
