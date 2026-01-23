// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TransitionScreen.generated.h"

/**
 * 
 */
UCLASS()
class CAPSTONE_API UTransitionScreen : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadStarted();

	UFUNCTION(BlueprintImplementableEvent)
	void OnLoadFinished();
};
