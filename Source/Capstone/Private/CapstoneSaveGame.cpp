// Fill out your copyright notice in the Description page of Project Settings.


#include "CapstoneSaveGame.h"

DEFINE_LOG_CATEGORY(LogPlayerAction);

void UCapstoneSaveGame::Initialize() 
{
    BattlesEntered = 0;
    BattlesWon = 0;
    MusicVolume = 1;
    SFXVolume = 1;
    CurrentLevel = FName();
    SaveLocation = FVector();
    SaveRotation = FQuat();
    ConsumedTriggers.Empty();
    Difficulty = 1;
};

void UCapstoneSaveGame::Reset() 
{
    BattlesEntered = 0;
    BattlesWon = 0;
    CurrentLevel = FName();
    SaveLocation = FVector();
    SaveRotation = FQuat();
    ConsumedTriggers.Empty();
    Difficulty = 1;
};

void UCapstoneSaveGame::LogBattleEntered()
{
    BattlesEntered++;
    UE_LOG(LogPlayerAction, Log, TEXT("Battle entry seen. Adding tally for a total of %d battles entered."), BattlesEntered)
}

void UCapstoneSaveGame::LogBattleWin()
{
    BattlesWon++;
    UE_LOG(LogPlayerAction, Log, TEXT("Battle win seen. Adding tally for a total of %d battles won."), BattlesEntered)
}

int UCapstoneSaveGame::GetBattlesEntered() const
{
    return BattlesEntered;
}

int UCapstoneSaveGame::GetBattlesWon() const
{
    return BattlesWon;
}

int UCapstoneSaveGame::GetBattlesLost() const
{
    return BattlesEntered - BattlesWon;
}

bool UCapstoneSaveGame::IsConsumed(FName Id) const
{
    return ConsumedTriggers.Contains(Id);
}

void UCapstoneSaveGame::Consume(FName Id)
{
    ConsumedTriggers.Add(Id);
}