#include "Combat/ANPCAIFC.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

#include "Combat/NpcTalkWidget.h"

ANPCAIFC::ANPCAIFC()
{
	PrimaryActorTick.bCanEverTick = true;

	/* =========================
	 * Interaction sphere
	 * ========================= */
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(GetCapsuleComponent());
	InteractionSphere->SetSphereRadius(180.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	/* =========================
	 * Character mesh setup
	 * ========================= */
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		SkelMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		SkelMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		SkelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkelMesh->SetGenerateOverlapEvents(false);
	}

	/* =========================
	 * World-space dialogue widget
	 * ========================= */
	DialogueWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DialogueWidgetComp"));
	DialogueWidgetComp->SetupAttachment(GetCapsuleComponent());
	DialogueWidgetComp->SetRelativeLocation(FVector::ZeroVector);
	DialogueWidgetComp->SetRelativeRotation(FRotator::ZeroRotator);
	DialogueWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	DialogueWidgetComp->SetDrawAtDesiredSize(true);
	DialogueWidgetComp->SetTwoSided(true);
	DialogueWidgetComp->SetVisibility(false, true);

	/* =========================
	 * Default dialogue content
	 * ========================= */
	NpcDisplayName = FText::FromString(TEXT("NPC"));
	DialogueLines =
	{
		FText::FromString(TEXT("Hello.")),
		FText::FromString(TEXT("Click again to continue.")),
		FText::FromString(TEXT("This is the last line."))
	};

	/* =========================
	 * State initialization
	 * ========================= */
	CurrentDialogueIndex = 0;
	bIsInDialogue = false;

	CurrentWaypointIndex = 0;
	WaitRemaining = 0.f;
	bIsMoving = false;

	bPlayerInRange = false;
	CachedPlayerPawn = nullptr;

	bPersistLastDialogueState = true;
	bDialogueCompleted = false;
	bLeftMouseWasDownLastFrame = false;
}

void ANPCAIFC::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionSphere)
	{
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANPCAIFC::OnPlayerEnterRange);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANPCAIFC::OnPlayerLeaveRange);
	}

	SetDialogueVisible(false);
	UpdateDialogueWidget();
}

void ANPCAIFC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/* =========================
	 * Handle left-click interaction while in range
	 * ========================= */
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PC && bPlayerInRange)
	{
		const bool bLeftMouseDownNow = PC->IsInputKeyDown(EKeys::LeftMouseButton);

		// Trigger only on the transition from "not pressed" to "pressed"
		if (bLeftMouseDownNow && !bLeftMouseWasDownLastFrame)
		{
			HandleLeftClickInteract();
		}

		bLeftMouseWasDownLastFrame = bLeftMouseDownNow;
	}
	else
	{
		bLeftMouseWasDownLastFrame = false;
	}

	/* =========================
	 * Keep widget above NPC head and facing camera
	 * ========================= */
	if (DialogueWidgetComp && DialogueWidgetComp->IsVisible())
	{
		const UCapsuleComponent* Cap = GetCapsuleComponent();
		if (Cap)
		{
			const FVector ActorLoc = GetActorLocation();
			const float HalfHeight = Cap->GetScaledCapsuleHalfHeight();
			const FVector HeadWorld = ActorLoc + FVector(0.f, 0.f, HalfHeight + 30.f);
			DialogueWidgetComp->SetWorldLocation(HeadWorld);
		}

		if (PC && PC->PlayerCameraManager)
		{
			const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
			const FVector WidgetLoc = DialogueWidgetComp->GetComponentLocation();
			const FRotator LookAtRot = (CamLoc - WidgetLoc).Rotation();
			DialogueWidgetComp->SetWorldRotation(FRotator(0.f, LookAtRot.Yaw, 0.f));
		}
	}

	// Stop movement while actively talking
	if (bIsInDialogue)
	{
		bIsMoving = false;
		return;
	}

	// Optional path movement logic
	if (!bEnablePathMove || Waypoints.Num() == 0)
	{
		bIsMoving = false;
		return;
	}

	// Wait at waypoint before moving again
	if (WaitRemaining > 0.f)
	{
		WaitRemaining = FMath::Max(0.f, WaitRemaining - DeltaSeconds);
		bIsMoving = false;
		return;
	}

	AActor* WP = Waypoints.IsValidIndex(CurrentWaypointIndex) ? Waypoints[CurrentWaypointIndex] : nullptr;
	if (!IsValid(WP))
	{
		CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
		bIsMoving = false;
		return;
	}

	const FVector Target = WP->GetActorLocation();
	const float Dist2D = FVector::Dist2D(GetActorLocation(), Target);

	if (Dist2D <= AcceptanceRadius)
	{
		WaitRemaining = WaitAtPointTime;
		CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
		bIsMoving = false;
		return;
	}

	bIsMoving = MoveConstantSpeedToward(Target, MoveSpeed, DeltaSeconds);
}

/* =========================
 * Overlap events
 * ========================= */

void ANPCAIFC::OnPlayerEnterRange(
	UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/,
	bool /*bFromSweep*/,
	const FHitResult& /*SweepResult*/
)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	if (!Pawn->IsPlayerControlled()) return;

	bPlayerInRange = true;
	CachedPlayerPawn = Pawn;

	// Always show the widget while the player is inside range
	SetDialogueVisible(true);

	// If dialogue was already completed before, show the last line directly
	if (bDialogueCompleted && DialogueLines.Num() > 0)
	{
		CurrentDialogueIndex = DialogueLines.Num() - 1;
		UpdateDialogueWidget();
		return;
	}

	// Otherwise, show the first line as preview when entering range
	if (!bIsInDialogue)
	{
		CurrentDialogueIndex = 0;
		UpdateDialogueWidget();
	}
}

void ANPCAIFC::OnPlayerLeaveRange(
	UPrimitiveComponent* /*OverlappedComp*/,
	AActor* OtherActor,
	UPrimitiveComponent* /*OtherComp*/,
	int32 /*OtherBodyIndex*/
)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	if (!Pawn->IsPlayerControlled()) return;

	if (CachedPlayerPawn == Pawn)
	{
		bPlayerInRange = false;
		CachedPlayerPawn = nullptr;

		// Leaving range always ends active dialogue
		EndDialogue();

		// Hide widget when leaving range if configured to do so
		if (bHideWidgetWhenOutOfRange)
		{
			SetDialogueVisible(false);
		}
	}
}

/* =========================
 * Left-click interaction
 * ========================= */

void ANPCAIFC::HandleLeftClickInteract()
{
	if (!bPlayerInRange) return;

	APawn* Interactor = CachedPlayerPawn;
	if (!Interactor)
	{
		APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		Interactor = PC ? PC->GetPawn() : nullptr;
		if (!Interactor) return;
	}

	Interact(Interactor);
}

/* =========================
 * Dialogue API
 * ========================= */

void ANPCAIFC::BeginDialogue(APawn* /*Interactor*/)
{
	// If already completed, keep showing the final line
	if (bDialogueCompleted && DialogueLines.Num() > 0)
	{
		bIsInDialogue = false;
		CurrentDialogueIndex = DialogueLines.Num() - 1;
		SetDialogueVisible(true);
		UpdateDialogueWidget();
		return;
	}

	bIsInDialogue = true;
	bIsMoving = false;
	CurrentDialogueIndex = 0;

	SetDialogueVisible(true);
	UpdateDialogueWidget();
}

void ANPCAIFC::EndDialogue()
{
	bIsInDialogue = false;
}

void ANPCAIFC::AdvanceDialogue()
{
	if (DialogueLines.Num() == 0)
	{
		EndDialogue();
		SetDialogueVisible(false);
		return;
	}

	// If dialogue is already marked complete, just keep showing the last line
	if (bDialogueCompleted)
	{
		CurrentDialogueIndex = DialogueLines.Num() - 1;
		UpdateDialogueWidget();
		return;
	}

	// Move to the next line if not at the last one yet
	if (CurrentDialogueIndex < DialogueLines.Num() - 1)
	{
		CurrentDialogueIndex++;
		UpdateDialogueWidget();
		return;
	}

	// If already on the last line and clicked again, mark as completed
	if (CurrentDialogueIndex == DialogueLines.Num() - 1)
	{
		if (bPersistLastDialogueState)
		{
			bDialogueCompleted = true;
			bIsInDialogue = false;
			UpdateDialogueWidget();
			return;
		}

		EndDialogue();
		SetDialogueVisible(false);
	}
}

void ANPCAIFC::UpdateDialogueWidget()
{
	UNpcTalkWidget* TalkWidget = GetTalkWidget();
	if (!TalkWidget) return;

	TalkWidget->SetNpcName(NpcDisplayName);

	if (DialogueLines.Num() == 0)
	{
		TalkWidget->SetLine(FText::GetEmpty());
		return;
	}

	const int32 SafeIndex = FMath::Clamp(CurrentDialogueIndex, 0, DialogueLines.Num() - 1);
	TalkWidget->SetLine(DialogueLines[SafeIndex]);
}

void ANPCAIFC::SetDialogueVisible(bool bVisible)
{
	if (!DialogueWidgetComp) return;
	DialogueWidgetComp->SetVisibility(bVisible, true);
}

void ANPCAIFC::Interact(APawn* Interactor)
{
	if (!Interactor) return;
	if (!bPlayerInRange) return;

	// If the dialogue was already completed before, keep showing the final line
	if (bDialogueCompleted && DialogueLines.Num() > 0)
	{
		CurrentDialogueIndex = DialogueLines.Num() - 1;
		SetDialogueVisible(true);
		UpdateDialogueWidget();
		return;
	}

	if (!bIsInDialogue)
	{
		BeginDialogue(Interactor);
		OnInteract(Interactor);
	}
	else
	{
		AdvanceDialogue();
	}
}

UNpcTalkWidget* ANPCAIFC::GetTalkWidget() const
{
	if (!DialogueWidgetComp) return nullptr;
	return Cast<UNpcTalkWidget>(DialogueWidgetComp->GetUserWidgetObject());
}

/* =========================
 * Movement helper
 * ========================= */

bool ANPCAIFC::MoveConstantSpeedToward(const FVector& Target, float Speed, float DeltaSeconds)
{
	const FVector Current = GetActorLocation();
	FVector To = Target - Current;
	To.Z = 0.f;

	const float Dist = To.Size();
	if (Dist <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector Dir = To / Dist;
	const float Step = Speed * DeltaSeconds;
	const FVector Next = Current + Dir * FMath::Min(Step, Dist);

	FHitResult Hit;
	const bool bSetOk = SetActorLocation(Next, true, &Hit, ETeleportType::None);
	const float Moved2D = FVector::Dist2D(Current, GetActorLocation());

	if (!bSetOk || Moved2D < MinMovedDistance)
	{
		return false;
	}

	return true;
}