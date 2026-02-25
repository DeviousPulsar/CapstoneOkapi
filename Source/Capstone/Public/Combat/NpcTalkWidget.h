#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NpcTalkWidget.generated.h"

class UTextBlock;

/**
 * Minimal world-space NPC dialogue widget used by a UWidgetComponent.
 * The text content is driven by C++ (ANPCAIFC), not manually typed in UMG.
 *
 * UMG requirements:
 * - Create a Widget Blueprint (e.g. WBP_NpcTalkWidget) with parent class UNpcTalkWidget.
 * - Add a TextBlock named "LineText" (required).
 * - Optionally add a TextBlock named "NameText".
 */
UCLASS()
class CAPSTONE_API UNpcTalkWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Sets the NPC display name (optional in UMG).
	UFUNCTION(BlueprintCallable, Category = "NPC|UI")
	void SetNpcName(const FText& InName);

	// Sets the dialogue line (required).
	UFUNCTION(BlueprintCallable, Category = "NPC|UI")
	void SetLine(const FText& InLine);

	// Clears both fields (helper).
	UFUNCTION(BlueprintCallable, Category = "NPC|UI")
	void Clear();

protected:
	// Optional name label. Bind to a TextBlock named "NameText" if present.
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* NameText = nullptr;

	// Required dialogue label. Bind to a TextBlock named "LineText".
	UPROPERTY(meta = (BindWidget))
	UTextBlock* LineText = nullptr;
};