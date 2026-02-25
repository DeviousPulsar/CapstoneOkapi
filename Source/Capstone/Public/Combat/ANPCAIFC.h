#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/EngineTypes.h"
#include "InputCoreTypes.h"

#include "ANPCAIFC.generated.h"

class USphereComponent;
class UWidgetComponent;
class UNpcTalkWidget;

UCLASS()
class CAPSTONE_API ANPCAIFC : public ACharacter
{
	GENERATED_BODY()

public:
	ANPCAIFC();

	// Called by player interaction (e.g., left-click trace from OverworldPawn).
	// If not in dialogue, starts it; otherwise advances to next line.
	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	void Interact(APawn* Interactor);

	// Starts dialogue mode (stops movement, shows first line).
	UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
	void BeginDialogue(APawn* Interactor);

	// Ends dialogue mode (hides widget if configured).
	UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
	void EndDialogue();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/* =========================
	 * Interaction (Near + Click)
	 * ========================= */

	UFUNCTION()
	void OnPlayerEnterRange(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnPlayerLeaveRange(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	// Click callback (requires trace to hit this actor/capsule).
	UFUNCTION()
	void OnNpcClicked(AActor* TouchedActor, FKey ButtonPressed);

	// Blueprint hook when player successfully interacts (optional).
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Interaction")
	void OnInteract(APawn* Interactor);

protected:
	UPROPERTY(VisibleAnywhere, Category = "NPC|Interaction")
	USphereComponent* InteractionSphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "NPC|Interaction")
	bool bPlayerInRange = false;

	UPROPERTY(VisibleAnywhere, Category = "NPC|Interaction")
	APawn* CachedPlayerPawn = nullptr;

	/* =========================
	 * 3D Dialogue UI (WidgetComponent)
	 * ========================= */

	 // World-space widget displayed above NPC head.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	UWidgetComponent* DialogueWidgetComp = nullptr;

	// Display name for the NPC.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	FText NpcDisplayName;

	// Dialogue lines driven by C++.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	TArray<FText> DialogueLines;

	// Auto-hide widget when player leaves interaction range.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	bool bHideWidgetWhenOutOfRange = true;

	// Current dialogue line index.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|UI")
	int32 CurrentDialogueIndex = 0;

	// Advance to next line / end when finished.
	void AdvanceDialogue();

	// Push name/line into the widget instance.
	void UpdateDialogueWidget();

	// Show/hide widget component safely.
	void SetDialogueVisible(bool bVisible);

	// Get typed widget instance from widget component.
	UNpcTalkWidget* GetTalkWidget() const;

	/* =========================
	 * Optional Path Movement
	 * ========================= */

	UPROPERTY(EditAnywhere, Category = "NPC|Move")
	bool bEnablePathMove = false;

	UPROPERTY(EditInstanceOnly, Category = "NPC|Move")
	TArray<AActor*> Waypoints;

	UPROPERTY(EditAnywhere, Category = "NPC|Move")
	float MoveSpeed = 180.f;

	UPROPERTY(EditAnywhere, Category = "NPC|Move")
	float AcceptanceRadius = 35.f;

	UPROPERTY(EditAnywhere, Category = "NPC|Move")
	float WaitAtPointTime = 0.5f;

	UPROPERTY(EditAnywhere, Category = "NPC|Move")
	float MinMovedDistance = 0.5f;

	/* =========================
	 * Anim placeholders
	 * ========================= */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Anim")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Anim")
	bool bIsInDialogue = false;

private:
	int32 CurrentWaypointIndex = 0;
	float WaitRemaining = 0.f;

	bool MoveConstantSpeedToward(const FVector& Target, float Speed, float DeltaSeconds);
};