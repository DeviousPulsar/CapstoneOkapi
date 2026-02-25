#include "Combat/ANPCAIFC.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

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

	// Allow mouse click trace to hit capsule via Visibility channel
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	/* =========================
	 * Preconfigure Character mesh (skeletal-ready, assets left empty)
	 * ========================= */
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		// Typical ACharacter setup: mesh is offset down so feet touch ground.
		// You can adjust this later depending on the asset you pick.
		SkelMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		SkelMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

		// Use animation blueprint mode by default; Anim Class can be assigned later.
		SkelMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);

		// Collision is usually handled by capsule; keep mesh non-blocking.
		SkelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkelMesh->SetGenerateOverlapEvents(false);
	}

	/* =========================
	 * World-space widget (3D UI)
	 * ========================= */
	DialogueWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DialogueWidgetComp"));
	DialogueWidgetComp->SetupAttachment(GetCapsuleComponent());

	// Do not hardcode head height here; we will lock world location in Tick.
	DialogueWidgetComp->SetRelativeLocation(FVector::ZeroVector);
	DialogueWidgetComp->SetRelativeRotation(FRotator::ZeroRotator);

	DialogueWidgetComp->SetWidgetSpace(EWidgetSpace::World);
	DialogueWidgetComp->SetDrawAtDesiredSize(true);
	DialogueWidgetComp->SetTwoSided(true);

	// Start hidden
	DialogueWidgetComp->SetVisibility(false, true);

	/* =========================
	 * Default dialogue data (editable in Details)
	 * ========================= */
	NpcDisplayName = FText::FromString(TEXT("NPC"));
	DialogueLines =
	{
		FText::FromString(TEXT("Hello.")),
		FText::FromString(TEXT("Click again to continue.")),
		FText::FromString(TEXT("This is the last line. Click to end.")),
	};

	/* =========================
	 * State init
	 * ========================= */
	CurrentDialogueIndex = 0;
	bIsInDialogue = false;

	CurrentWaypointIndex = 0;
	WaitRemaining = 0.f;
	bIsMoving = false;

	bPlayerInRange = false;
	CachedPlayerPawn = nullptr;
}

void ANPCAIFC::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionSphere)
	{
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ANPCAIFC::OnPlayerEnterRange);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ANPCAIFC::OnPlayerLeaveRange);
	}

	// Actor click callback
	OnClicked.AddDynamic(this, &ANPCAIFC::OnNpcClicked);

	// Ensure correct initial UI
	SetDialogueVisible(false);
	UpdateDialogueWidget();
}

void ANPCAIFC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/* =========================
	 * Lock widget to head-top + billboard to camera (yaw only)
	 * ========================= */
	if (DialogueWidgetComp && DialogueWidgetComp->IsVisible())
	{
		// 1) Lock position to capsule top in WORLD space (guaranteed centered on actor)
		const UCapsuleComponent* Cap = GetCapsuleComponent();
		if (Cap)
		{
			const FVector ActorLoc = GetActorLocation();
			const float HalfHeight = Cap->GetScaledCapsuleHalfHeight();

			// Actor origin is at capsule center; move up to capsule top + extra lift
			const FVector HeadWorld = ActorLoc + FVector(0.f, 0.f, HalfHeight + 30.f);
			DialogueWidgetComp->SetWorldLocation(HeadWorld);
		}

		// 2) Billboard rotation (yaw only)
		APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		if (PC && PC->PlayerCameraManager)
		{
			const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
			const FVector WidgetLoc = DialogueWidgetComp->GetComponentLocation();
			const FRotator LookAtRot = (CamLoc - WidgetLoc).Rotation();
			DialogueWidgetComp->SetWorldRotation(FRotator(0.f, LookAtRot.Yaw, 0.f));
		}
	}

	// Stop movement while in dialogue
	if (bIsInDialogue)
	{
		bIsMoving = false;
		return;
	}

	// Optional path movement
	if (!bEnablePathMove || Waypoints.Num() == 0)
	{
		bIsMoving = false;
		return;
	}

	// Waiting at waypoint
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

	// Show widget when player is close
	SetDialogueVisible(true);

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

		if (bHideWidgetWhenOutOfRange)
		{
			EndDialogue();
			SetDialogueVisible(false);
		}
	}
}

/* =========================
 * Click interaction
 * ========================= */

void ANPCAIFC::OnNpcClicked(AActor* TouchedActor, FKey /*ButtonPressed*/)
{
	if (TouchedActor != this) return;
	if (!bPlayerInRange) return;

	APawn* Interactor = CachedPlayerPawn;
	if (!Interactor)
	{
		APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
		Interactor = PC ? PC->GetPawn() : nullptr;
		if (!Interactor) return;
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

/* =========================
 * Dialogue API
 * ========================= */

void ANPCAIFC::BeginDialogue(APawn* /*Interactor*/)
{
	bIsInDialogue = true;
	bIsMoving = false;

	CurrentDialogueIndex = 0;

	SetDialogueVisible(true);
	UpdateDialogueWidget();
}

void ANPCAIFC::EndDialogue()
{
	bIsInDialogue = false;

	if (!bPlayerInRange && bHideWidgetWhenOutOfRange)
	{
		SetDialogueVisible(false);
	}
}

void ANPCAIFC::AdvanceDialogue()
{
	if (DialogueLines.Num() == 0)
	{
		EndDialogue();
		SetDialogueVisible(false);
		return;
	}

	CurrentDialogueIndex++;

	if (CurrentDialogueIndex >= DialogueLines.Num())
	{
		EndDialogue();
		SetDialogueVisible(false);
		return;
	}

	UpdateDialogueWidget();
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