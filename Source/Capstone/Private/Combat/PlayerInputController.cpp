// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/PlayerInputController.h"
#include "Combat/CombatPawn.h"
#include "InputAction.h"

APlayerInputController::APlayerInputController()
{
	
}

void APlayerInputController::BeginPlay()
{
	Super::BeginPlay();

    
	
}

void APlayerInputController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // TODO: figure out how to connect to pawn and have a callback on the pawn move method
}

