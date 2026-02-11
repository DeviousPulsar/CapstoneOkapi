// PatrolAICharacter.cpp

#include "Combat/PatrolAICharacter.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Overworld/CombatTransitionTrigger.h"

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

	if (DetectSphere)
	{
		DetectSphere->OnComponentBeginOverlap.AddDynamic(this, &APatrolAICharacter::OnDetectPlayer);
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	PlayerPawn = PC ? PC->GetPawn() : nullptr;

	GetWorldTimerManager().SetTimer(
		PatrolTimerHandle,
		this,
		&APatrolAICharacter::PatrolSelectTarget,
		SelectInterval,
		true
	);

	PatrolSelectTarget();
}

void APatrolAICharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PlayerPawn)
	{
		const float DistToPlayer = FVector::Dist2D(GetActorLocation(), PlayerPawn->GetActorLocation());

		if (!bChasingPlayer && DistToPlayer <= ChaseRadius)
		{
			bChasingPlayer = true;
		}

		if (bChasingPlayer)
		{
			if (DistToPlayer > LoseRadius)
			{
				bChasingPlayer = false;
				PatrolSelectTarget();
			}
			else
			{
				if (DistToPlayer <= TriggerDistance)
				{
					if (!(bTriggerOnce && bHasTriggered))
					{
						if (TransitionTrigger)
						{
							bHasTriggered = true;
							TransitionTrigger->BeginTransition();
						}
					}
					return;
				}

				CurrentTarget = PlayerPawn;
			}
		}
	}

	if (!CurrentTarget) return;

	const FVector Current = GetActorLocation();
	const FVector Destination = CurrentTarget->GetActorLocation();

	if (FVector::Dist2D(Current, Destination) <= AcceptanceRadius) return;

	FVector Dir = Destination - Current;
	Dir.Z = 0.f;
	if (!Dir.Normalize()) return;

	AddMovementInput(Dir, MoveInputScale);
}

void APatrolAICharacter::PatrolSelectTarget()
{
	if (PatrolPoints.Num() == 0)
	{
		CurrentTarget = nullptr;
		return;
	}

	if (bChasingPlayer) return;

	const int32 N = PatrolPoints.Num();
	if (N == 1)
	{
		PatrolIndex = 0;
		CurrentTarget = PatrolPoints[0];
		return;
	}

	int32 NewIndex = PatrolIndex;
	int32 Guard = 0;
	while (NewIndex == PatrolIndex && Guard < 10)
	{
		NewIndex = FMath::RandRange(0, N - 1);
		Guard++;
	}

	PatrolIndex = NewIndex;
	CurrentTarget = PatrolPoints[PatrolIndex];
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

	if (FVector::Dist2D(GetActorLocation(), Pawn->GetActorLocation()) <= TriggerDistance)
	{
		if (bTriggerOnce && bHasTriggered) return;

		if (TransitionTrigger)
		{
			bHasTriggered = true;
			TransitionTrigger->BeginTransition();
		}
	}
}
