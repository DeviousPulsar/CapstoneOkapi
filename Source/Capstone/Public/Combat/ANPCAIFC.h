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

	// Called when the player interacts with this NPC.
	// If dialogue has not started, it begins.
	// If dialogue is already active, it advances.
	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	void Interact(APawn* Interactor);

	// Starts dialogue mode and shows the first relevant line.
	UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
	void BeginDialogue(APawn* Interactor);

	// Ends dialogue mode. Widget visibility is still controlled by range rules.
	UFUNCTION(BlueprintCallable, Category = "NPC|Interaction")
	void EndDialogue();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/* =========================
	 * Interaction (Near + Left Click)
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

	// Handles left-click interaction while the player is inside range.
	UFUNCTION()
	void HandleLeftClickInteract();

	// Optional Blueprint event hook for custom interaction responses.
	UFUNCTION(BlueprintImplementableEvent, Category = "NPC|Interaction")
	void OnInteract(APawn* Interactor);

protected:
	/* =========================
	 * Interaction state
	 * ========================= */

	 // Overlap sphere used to detect whether the player is close enough to interact.
	UPROPERTY(VisibleAnywhere, Category = "NPC|Interaction")
	USphereComponent* InteractionSphere = nullptr;

	// True if the player is currently inside the interaction sphere.
	UPROPERTY(VisibleAnywhere, Category = "NPC|Interaction")
	bool bPlayerInRange = false;

	// Cached player pawn currently interacting or inside the range.
	UPROPERTY(VisibleAnywhere, Category = "NPC|Interaction")
	APawn* CachedPlayerPawn = nullptr;

	/* =========================
	 * 3D Dialogue UI
	 * ========================= */

	 // World-space widget shown above the NPC.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	UWidgetComponent* DialogueWidgetComp = nullptr;

	// NPC display name shown in the dialogue widget.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	FText NpcDisplayName;

	// Dialogue lines for this NPC.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	TArray<FText> DialogueLines;

	// If true, the widget hides when the player leaves range.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	bool bHideWidgetWhenOutOfRange = true;

	// If true, clicking once more on the last line marks the dialogue as completed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|UI")
	bool bPersistLastDialogueState = true;

	// True after this NPC's dialogue has been completed during the current play session.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|UI")
	bool bDialogueCompleted = false;

	// Current dialogue line index being displayed.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|UI")
	int32 CurrentDialogueIndex = 0;

	// Advances dialogue, completes state if needed, or ends conversation.
	void AdvanceDialogue();

	// Pushes the current name and line into the widget.
	void UpdateDialogueWidget();

	// Safely shows or hides the widget component.
	void SetDialogueVisible(bool bVisible);

	// Returns the typed widget instance from the widget component.
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
	 * Animation / state flags
	 * ========================= */

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Anim")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Anim")
	bool bIsInDialogue = false;

private:
	int32 CurrentWaypointIndex = 0;
	float WaitRemaining = 0.f;

	bool MoveConstantSpeedToward(const FVector& Target, float Speed, float DeltaSeconds);

	// Used to prevent repeated interaction while holding down left mouse button.
	bool bLeftMouseWasDownLastFrame = false;
};