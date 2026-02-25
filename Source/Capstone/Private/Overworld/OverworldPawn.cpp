// Fill out your copyright notice in the Description page of Project Settings.


#include "Overworld/OverworldPawn.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Combat/ANPCAIFC.h"
#include "DrawDebugHelpers.h"

// Sets default values
AOverworldPawn::AOverworldPawn()
{
 	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AOverworldPawn::BeginPlay()
{
	Super::BeginPlay();

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

void AOverworldPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOverworldPawn::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOverworldPawn::Look);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AOverworldPawn::Interact);

	
		UE_LOG(LogTemp, Log, TEXT("Enhanced input component '%s' configured for '%s'"), *GetNameSafe(EnhancedInputComponent), *GetNameSafe(this));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AOverworldPawn::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AOverworldPawn::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AOverworldPawn::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Does not have a controller!"), *GetNameSafe(this));
	}
}

void AOverworldPawn::DoLook(float Yaw, float Pitch)
{
	Pitch = -Pitch;
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Does not have a controller!"), *GetNameSafe(this));
	}
}

void AOverworldPawn::Interact()
{
	UE_LOG(LogTemp, Warning, TEXT("Interact() fired"));

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// 1) Build a ray from the center of the screen (crosshair style)
	int32 SizeX = 0, SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);

	const FVector2D ScreenCenter(SizeX * 0.5f, SizeY * 0.5f);

	FVector WorldOrigin;
	FVector WorldDir;
	if (!PC->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldOrigin, WorldDir))
	{
		return;
	}

	// 2) Trace forward
	const float TraceDistance = 600.f;
	const FVector Start = WorldOrigin;
	const FVector End = Start + WorldDir * TraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	// Debug (optional)
	// DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.5f);

	if (!bHit) return;

	// 3) If hit an NPC, advance dialogue
	AActor* HitActor = Hit.GetActor();
	ANPCAIFC* Npc = Cast<ANPCAIFC>(HitActor);
	if (!Npc) return;

	// Your NPC already has range gating (bPlayerInRange) and dialogue logic.
	Npc->Interact(this);
}