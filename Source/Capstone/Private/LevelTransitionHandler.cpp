// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelTransitionHandler.h"
#include "GameFramework/Character.h"
#include "Internationalization/Regex.h"
#include "Kismet/GameplayStatics.h"



ULevelTransitionHandler::ULevelTransitionHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    OverworldMap = TEXT("");
    ReturnPosition = FVector(0, 0, 230);
    ReturnRotation = FQuat(1, 0, 0, 0);

    bInCombat = false;
}

void ULevelTransitionHandler::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
  Super::OnWorldChanged(OldWorld, NewWorld);
}

void ULevelTransitionHandler::LoadComplete(const float LoadTime, const FString& MapName)
{
    Super::LoadComplete(LoadTime, MapName);

    if(!bInCombat)
    {
        if(ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
        {
            Player->GetRootComponent()->SetWorldLocationAndRotation(ReturnPosition, ReturnRotation);
        }
    }
}

void ULevelTransitionHandler::LoadCombatScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation)
{
    bInCombat = true;
    UWorld* World = GetWorld();
    
    const FRegexPattern Pattern(TEXT("Level_.*"), ERegexPatternFlags::None);
    FRegexMatcher Regex(Pattern, World->GetName());
    if (Regex.FindNext())
    {
        OverworldMap = FName(Regex.GetCaptureGroup(0));
    }
    else
    {
        UE_LOG(LogLoad, Error, TEXT("'%s' does not contain a valid level name"), *LevelToLoad.ToString());
        return;
    }
    
    ReturnPosition = pReturnPosition;
    ReturnRotation = pReturnRotation;

    UE_LOG(
        LogLoad, 
        Log, 
        TEXT("Moving to Level %s from level %s. Player actor will return with position %s and facing %s"), 
        *LevelToLoad.ToString(), 
        *OverworldMap.ToString(), 
        *ReturnPosition.ToString(), 
        *ReturnRotation.Rotator().ToString()
    );

    UGameplayStatics::OpenLevel(World, LevelToLoad);
}

void ULevelTransitionHandler::LoadOverworldScene(const FName LevelToLoad, const FVector DestinationPosition, const FQuat DestinationRotation)
{
    bInCombat = false;
    const FRegexPattern Pattern(TEXT("Level_.*"), ERegexPatternFlags::None);
    FRegexMatcher Regex(Pattern, LevelToLoad.ToString());
    if (Regex.FindNext())
    {
        OverworldMap = FName(Regex.GetCaptureGroup(0));
    }
    else
    {
        UE_LOG(LogLoad, Error, TEXT("'%s' does not contain a valid level name"), *LevelToLoad.ToString());
        return;
    }

    ReturnPosition = DestinationPosition;
    ReturnRotation = DestinationRotation;

    UWorld* World = GetWorld();

    UE_LOG(
        LogLoad, 
        Log, 
        TEXT("Moving to Level %s from level %s. Player actor, if one exists, will be placed at %s with facing %s"), 
        *LevelToLoad.ToString(), 
        *FName(World->GetMapName()).ToString(), 
        *ReturnPosition.ToString(), 
        *ReturnRotation.Rotator().ToString()
    )

    UGameplayStatics::OpenLevel(World, LevelToLoad);
}

void ULevelTransitionHandler::ReturnToOverworld()
{
    bInCombat = false;
    UWorld* World = GetWorld();

    UE_LOG(
        LogLoad, 
        Log, 
        TEXT("Moving to Level %s from level %s. Player actor, if one exists, will be placed at %s with facing %s"), 
        *OverworldMap.ToString(), 
        *FName(World->GetMapName()).ToString(),  
        *ReturnPosition.ToString(), 
        *ReturnRotation.Rotator().ToString()
    );

    UGameplayStatics::OpenLevel(World, OverworldMap);
}