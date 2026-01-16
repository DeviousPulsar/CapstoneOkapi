// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerTrackingSubsystem.h"

DEFINE_LOG_CATEGORY(LogPlayerAction);

void UPlayerTrackingSubsystem::Initialize(FSubsystemCollectionBase& Collection) 
{
    BattlesEntered = 0;
    BattlesWon = 0;
};

void UPlayerTrackingSubsystem::Deinitialize() { };

void UPlayerTrackingSubsystem::LogBattleEntered()
{
    BattlesEntered++;
    UE_LOG(LogPlayerAction, Log, TEXT("Battle entry seen. Adding tally for a total of %d battles entered."), BattlesEntered)
}

void UPlayerTrackingSubsystem::LogBattleWin()
{
    BattlesWon++;
    UE_LOG(LogPlayerAction, Log, TEXT("Battle win seen. Adding tally for a total of %d battles won."), BattlesEntered)
}

int UPlayerTrackingSubsystem::GetBattlesEntered() const
{
    return BattlesEntered;
}

int UPlayerTrackingSubsystem::GetBattlesWon() const
{
    return BattlesWon;
}

int UPlayerTrackingSubsystem::GetBattlesLost() const
{
    return BattlesEntered - BattlesWon;
}