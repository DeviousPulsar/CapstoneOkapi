// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "InputAction.h"
#include "InputMappingContext.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerInputController.generated.h"


/**
 * 
 */
UCLASS()
class CAPSTONE_API APlayerInputController : public APlayerController
{
	GENERATED_BODY()

public:

	APlayerInputController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* PawnMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;
};
