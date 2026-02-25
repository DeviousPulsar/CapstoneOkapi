#include "Overworld/PatrolAICharacter.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "Overworld/CombatTransitionTrigger.h"
#include "LevelTransitionHandler.h"

APatrolAICharacter::APatrolAICharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectSphere"));
	DetectSphere->SetupAttachment(GetCapsuleComponent());
	DetectSphere->SetSphereRadius(150.f);
	DetectSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void APatrolAICharacter::BeginPlay()
{
	Super::BeginPlay();

	// If no explicit id is set in editor, default to actor name (unique enough per map instance)
	if (PersistentId.IsNone())
	{
		PersistentId = GetFName();
	}

	/* Session-only consumption check (NO SaveGame)
	if (ULevelTransitionHandler* TH = Cast<ULevelTransitionHandler>(GetGameInstance()))
	{
		if (TH->IsActorConsumedSession(PersistentId))
		{
			Destroy();
			return;
		}
	}*/

	if (DetectSphere)
	{
		DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &APatrolAICharacter::OnDetectPlayer);
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	PlayerPawn = PC ? PC->GetPawn() : nullptr;

	SpawnLocation = GetActorLocation();

	// Initialize patrol target
	RefreshCruiseTarget();
}

void APatrolAICharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Cooldowns to avoid rapid retargeting flicker
	if (RetargetCooldown > 0.f) RetargetCooldown = FMath::Max(0.f, RetargetCooldown - DeltaSeconds);
	if (ReachedHold > 0.f)      ReachedHold = FMath::Max(0.f, ReachedHold - DeltaSeconds);

	// --- Update chase state based on distance ---
	if (PlayerPawn)
	{
		const float DistToPlayer = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());

		// Enter chase when close enough
		if (!bChasingPlayer && DistToPlayer <= ChaseRadius)
		{
			bChasingPlayer = true;
			// Reset patrol stabilizers when switching modes
			StuckTime = 0.f;
			ReachedHold = 0.f;
			RetargetCooldown = 0.f;
		}

		if (bChasingPlayer)
		{
			// Lose player -> back to patrol
			if (DistToPlayer > LoseRadius)
			{
				bChasingPlayer = false;

				// Choose a fresh patrol target (with a small cooldown to prevent instant flip-flop)
				RetargetCooldown = RetargetCooldownTime;
				ReachedHold = ReachedHoldTime;
				StuckTime = 0.f;
				RefreshCruiseTarget();
			}
			else
			{
				// Trigger transition if close enough
				if (DistToPlayer <= TriggerDistance)
				{
					if (!(bTriggerOnce && bHasTriggered) && TransitionTrigger)
					{
						bHasTriggered = true;

						// Mark consumed for this session only, then transition, then destroy
						if (ULevelTransitionHandler* TH = Cast<ULevelTransitionHandler>(GetGameInstance()))
						{
							if (PersistentId.IsNone())
							{
								PersistentId = GetFName();
							}
							//TH->ConsumeActorSession(PersistentId);
						}

						TransitionTrigger->BeginTransition();
						Destroy();
					}
					return;
				}

				// Constant-speed chase.
				// If we get stuck (collision), we do NOT retarget (target is the player).
				MoveConstantSpeedToward(PlayerPawn->GetActorLocation(), ChaseSpeed, DeltaSeconds);
				return;
			}
		}
	}

	// --- Patrol (constant speed within PatrolAreaActor bounds) ---
	if (!bHasCruiseTarget)
	{
		RefreshCruiseTarget();
	}

	// If we just reached a point, hold briefly to avoid micro-oscillation near the acceptance radius
	if (ReachedHold > 0.f)
	{
		return;
	}

	const float DistToTarget = FVector::Dist2D(GetActorLocation(), CruiseTarget);

	// Arrived at patrol target -> pick a new one, but apply a cooldown to avoid rapid flipping
	if (DistToTarget <= AcceptanceRadius)
	{
		if (RetargetCooldown <= 0.f)
		{
			RetargetCooldown = RetargetCooldownTime;
			ReachedHold = ReachedHoldTime;
			StuckTime = 0.f;
			RefreshCruiseTarget();
		}
		return;
	}

	// Move and detect "stuck" (collision / blocked movement). If stuck for too long, pick a new target.
	const bool bMoved = MoveConstantSpeedToward(CruiseTarget, CruiseSpeed, DeltaSeconds);
	if (!bMoved)
	{
		StuckTime += DeltaSeconds;

		// If blocked for a short period, retarget (with cooldown) to avoid flicker
		if (StuckTime >= StuckTimeThreshold && RetargetCooldown <= 0.f)
		{
			RetargetCooldown = RetargetCooldownTime;
			ReachedHold = 0.f;
			StuckTime = 0.f;
			RefreshCruiseTarget();
		}
	}
	else
	{
		// Successful move -> reset stuck timer
		StuckTime = 0.f;
	}
}

void APatrolAICharacter::OnDetectPlayer(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	int32,
	bool,
	const FHitResult&
)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	if (!Pawn->IsPlayerControlled()) return;

	bChasingPlayer = true;
	PlayerPawn = Pawn;

	// Reset patrol stabilizers when switching modes
	StuckTime = 0.f;
	ReachedHold = 0.f;
	RetargetCooldown = 0.f;

	// Immediate trigger if already close enough
	if (FVector::Dist2D(GetActorLocation(), Pawn->GetActorLocation()) <= TriggerDistance)
	{
		if (bTriggerOnce && bHasTriggered) return;

		if (TransitionTrigger)
		{
			bHasTriggered = true;

			// Mark consumed for this session only, then transition, then destroy
			if (ULevelTransitionHandler* TH = Cast<ULevelTransitionHandler>(GetGameInstance()))
			{
				if (PersistentId.IsNone())
				{
					PersistentId = GetFName();
				}
				//TH->ConsumeActorSession(PersistentId);
			}

			TransitionTrigger->BeginTransition();
			Destroy();
		}
	}
}

FVector APatrolAICharacter::PickPointInPatrolArea() const
{
	// If a patrol area actor is assigned (TriggerBox / any actor with bounds)
	if (PatrolAreaActor)
	{
		FVector Origin, Extent;
		PatrolAreaActor->GetActorBounds(true, Origin, Extent);

		const float X = FMath::FRandRange(Origin.X - Extent.X, Origin.X + Extent.X);
		const float Y = FMath::FRandRange(Origin.Y - Extent.Y, Origin.Y + Extent.Y);

		// Keep patrol on same Z plane as AI (typical overworld)
		return FVector(X, Y, GetActorLocation().Z);
	}

	// Fallback: roam around spawn point in a circle-ish range
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float Radius = FMath::FRandRange(0.f, FallbackRadius);
	const float X = SpawnLocation.X + FMath::Cos(Angle) * Radius;
	const float Y = SpawnLocation.Y + FMath::Sin(Angle) * Radius;
	return FVector(X, Y, GetActorLocation().Z);
}

void APatrolAICharacter::RefreshCruiseTarget()
{
	CruiseTarget = PickPointInPatrolArea();
	bHasCruiseTarget = true;
}

bool APatrolAICharacter::MoveConstantSpeedToward(const FVector& Target, float Speed, float DeltaSeconds)
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

	// Avoid overshoot
	const FVector Next = Current + Dir * FMath::Min(Step, Dist);

	// sweep=true: respects collision (won't tunnel through walls if collision is set up)
	// Track whether we actually moved (to detect being blocked).
	FHitResult Hit;
	const bool bSetOk = SetActorLocation(Next, true, &Hit, ETeleportType::None);
	const float Moved2D = FVector::Dist2D(Current, GetActorLocation());

	// If SetActorLocation failed or movement is extremely small, treat as blocked
	if (!bSetOk || Moved2D < MinMovedDistance)
	{
		return false;
	}

	return true;
}
