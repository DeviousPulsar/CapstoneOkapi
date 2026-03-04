// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatPlayer.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// Sets default values
ACombatPlayer::ACombatPlayer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//default values
	TimeSinceAttack = 10.0f;
	AttackCooldown = 0;
	AttackAllowed = true;
	Focus = EFocus::Default;
}

void ACombatPlayer::BeginPlay()
{
	Super::BeginPlay();

	//Spawn effects
	//UNiagaraSystem*, Duration, Scale
	AttackComponent = SpawnEffect(AttackVFX, 1);
	DeactivateEffect(AttackComponent);
}

void ACombatPlayer::PreviewAttack(EPlayerAttacks Attack)
{
	if (!Grid)
	{
		return;
	}

	Grid->ClearPlayerPreview();

	if (Attack == EPlayerAttacks::NoAttack)
	{
		return;
	}
	if (AttackMapping.Contains(Attack) == false)
	{
		return;
	}

	TSubclassOf<UAttack> AttackClass = *AttackMapping.Find(Attack);

	UAttack* AttackInstance = NewObject<UAttack>(this, AttackClass->GetFName(), RF_NoFlags, AttackClass.GetDefaultObject());
	if (!AttackInstance)
	{
		return;
	}

	if (AttackInstance->bDynamic)
	{
		AttackInstance = AttackInstance->AsStaticAttack(GetPosition().x, GetPosition().y);
		if (!AttackInstance)
		{
			return;
		}
	}

	TSet<FIntPoint> Unique;
	TArray<FGridPosition> Tiles;

	for (const FAttackStage& Stage : AttackInstance->AttackStages)
	{
		for (const FGridPosition& P : Stage.Targets)
		{
			FIntPoint Key(P.x, P.y);
			if (!Unique.Contains(Key))
			{
				Unique.Add(Key);
				Tiles.Add(P);
			}
		}
	}

	Grid->ShowPlayerPreview(Tiles);
}

// Called when the game starts or when spawned
void ACombatPlayer::Restart()
{
	Super::Restart();

	if (APlayerController* controller = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(controller->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("'%s' Unable to add Mapping Context!"), *GetNameSafe(this));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Unable to add Mapping Context! Invalid Controller!"), *GetNameSafe(this));
	}
}

// Called every frame
void ACombatPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSinceAttack += DeltaTime;
	if (TimeSinceAttack >= AttackCooldown) {
		AttackAllowed = true;
		DeactivateEffect(AttackComponent);
	}

	if (AttackAllowed && !bIsFrozen)
	{
		PreviewAttack(LeftClickAttack);
	}
	else if (Grid)
	{
		Grid->ClearPlayerPreview();
	}
}

TArray<EPlayerAttacks> ACombatPlayer::GetAvailablePlayerAttacks() {
	return AvailablePlayerAttacks;
}

void ACombatPlayer::AddAvailablePlayerAttack(EPlayerAttacks Attack) {
	AvailablePlayerAttacks.Add(Attack);
}

void ACombatPlayer::AttackGrid(EPlayerAttacks Attack)
{
	if (!AttackAllowed || bIsFrozen)
	{
		return;
	}
	if (Attack == EPlayerAttacks::NoAttack)
	{
		return;
	}
	if (AttackMapping.Contains(Attack) == false)
	{
		return;
	}

	TSubclassOf<UAttack> AttackClass = *AttackMapping.Find(Attack);

	UAttack* AttackInstance = NewObject<UAttack>(this, AttackClass->GetFName(), RF_NoFlags, AttackClass.GetDefaultObject());
	if (!AttackInstance)
	{
		return;
	}

	if (AttackInstance->bDynamic)
	{
		AttackInstance = AttackInstance->AsStaticAttack(GetPosition().x, GetPosition().y);
	}

	if (Focus == EFocus::Attack)
	{
		AttackInstance->Buff(DamageBuff);
	}

	if (ParryBoost)
	{
		if (Focus != EFocus::Parry)
		{
			AttackInstance->Buff(ParryDamageBuff);
		}
		else
		{
			AttackInstance->Buff(StrongAttackParryBuff);
		}

		// change bParriable so it uses the strong attack vfx
		for (FAttackStage& currentFrame : AttackInstance->AttackStages){
			currentFrame.bParriable = false;
		}
		
		ParryBoost = false;
		DeactivateEffect(ParryBoostComponent);
	}

	Grid->ExecuteAttack(AttackInstance, true);
	Grid->ClearPlayerPreview();

	AttackCooldown = AttackInstance->Cooldown;
	TimeSinceAttack = 0.0f;
	AttackAllowed = false;
	PlayAttackMontage();
	ActivateEffect(AttackComponent);
	Stun(AttackInstance->UseTime);
}

void ACombatPlayer::ChangeLeftClickAttack(EPlayerAttacks NewAttack) {
	LeftClickAttack = NewAttack;
	PreviewAttack(LeftClickAttack);
}

void ACombatPlayer::ChangeRightClickAttack(EPlayerAttacks NewAttack) {
	RightClickAttack = NewAttack;
	PreviewAttack(LeftClickAttack);
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
		SetDefend(DefenseBuff);
		break;
	}
	case EFocus::Heal:
	{
		//Pawn->EditHealth(HealBuff); Moved to SetMovementAllowed
		break;
	}
	case EFocus::Parry:
	{
		SetParryTimeBuff(ParryWindowBuff);
		break;
	}
	case EFocus::Default:
	{
		SetDefend(1.0);
		SetParryTimeBuff(0);
		break;
	}
	}
}

EFocus ACombatPlayer::GetBuff()
{
	return Focus;
}

void ACombatPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	Super::Move(MovementVector);
	PreviewAttack(LeftClickAttack);
}

void ACombatPlayer::Attack()
{
	AttackGrid(LeftClickAttack);
}

void ACombatPlayer::Parry()
{
	AttemptParry();
}

void ACombatPlayer::OptionMenu()
{
	OnOptionsSignal.Broadcast();
}

void ACombatPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(UpAction, ETriggerEvent::Started, this, &ACombatPlayer::UpPressed);
		EnhancedInputComponent->BindAction(DownAction, ETriggerEvent::Started, this, &ACombatPlayer::DownPressed);
		EnhancedInputComponent->BindAction(LeftAction, ETriggerEvent::Started, this, &ACombatPlayer::LeftPressed);
		EnhancedInputComponent->BindAction(RightAction, ETriggerEvent::Started, this, &ACombatPlayer::RightPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ACombatPlayer::Attack);
		EnhancedInputComponent->BindAction(ParryAction, ETriggerEvent::Started, this, &ACombatPlayer::Parry);
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ACombatPlayer::OptionMenu);

		UE_LOG(LogTemp, Log, TEXT("Enhanced input component '%s' configured for '%s'"), *GetNameSafe(EnhancedInputComponent), *GetNameSafe(this));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}
