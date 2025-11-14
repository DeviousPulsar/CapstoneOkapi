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
	if (!AttackAllowed || bIsFrozen)
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
		AttackInstance = AttackInstance->AsStaticAttack(GetPosition().x, GetPosition().y);
	}
	
	if (Focus == EFocus::Attack)
	{
		AttackInstance->Buff(DamageBuff);
	}

	if (ParryBoost)
	{
		AttackInstance->Buff(ParryDamageBuff);
		ParryBoost = false;
	}

	Grid->ExecuteAttack(AttackInstance);

	AttackCooldown = AttackInstance->Cooldown;
	TimeSinceAttack = 0.0f;
	AttackAllowed = false;
	Stun(AttackInstance->UseTime);
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
			SetDefend(DefenseBuff);
			break;
		}
		case EFocus::Heal:
		{
			//Pawn->EditHealth(HealBuff); Moved to SetMovementAllowed
			break;
		}
		case EFocus::Default:
		{
			SetDefend(0);
			break;
		}
	}
}

void ACombatPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	Super::Move(MovementVector);
}

void ACombatPlayer::Attack()
{
	PlayAttackMontage();
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ACombatPlayer::Move);
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