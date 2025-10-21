// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/Character.h"
#include "Internationalization/Regex.h"
#include "Kismet/GameplayStatics.h"

#include "LevelTransitionHandler.h"

UGameInstance* ULevelTransitionHandler::GetGameInstance()
{
    FWorldContext* world = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
    return UGameplayStatics::GetGameInstance(world->World());
}

UWorld* ULevelTransitionHandler::GetCurrentWorld()
{
   return GetGameInstance()->GetWorld();
}

void ULevelTransitionHandler::Initialize(FSubsystemCollectionBase& Collection) 
{
    Super::Initialize(Collection);

    OverworldMap = TEXT("");
    ReturnPosition = FVector(0, 0, 0);
}

void ULevelTransitionHandler::Deinitialize() {}

void ULevelTransitionHandler::LoadCombatScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation)
{
    UWorld* World = ULevelTransitionHandler::GetCurrentWorld();
    
    const FRegexPattern Pattern(TEXT("Level_.*"), ERegexPatternFlags::None);
    FRegexMatcher Regex(Pattern, World->GetMapName());
    if (Regex.FindNext())
    {
        OverworldMap = FName(Regex.GetCaptureGroup(0));
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
    const FRegexPattern Pattern(TEXT("Level_.*"), ERegexPatternFlags::None);
    FRegexMatcher Regex(Pattern, LevelToLoad.ToString());
    if (Regex.FindNext())
    {
        OverworldMap = FName(Regex.GetCaptureGroup(0));
    }

    ReturnPosition = DestinationPosition;
    ReturnRotation = DestinationRotation;

    UWorld* World = ULevelTransitionHandler::GetCurrentWorld();

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

    World = ULevelTransitionHandler::GetCurrentWorld();
    if(ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0))
    {
        Player->GetRootComponent()->SetWorldLocationAndRotation(DestinationPosition, DestinationRotation);
    }
}

void ULevelTransitionHandler::ReturnToOverworld()
{
    UWorld* World = ULevelTransitionHandler::GetCurrentWorld();

    UE_LOG(LogLoad, Log, TEXT("Moving to Level %s from level %s. Player actor, if one exists, will be placed at %s with facing %s"), *OverworldMap.ToString(), *FName(World->GetMapName()).ToString(),  *ReturnPosition.ToString(), *ReturnRotation.Rotator().ToString())

    UGameplayStatics::OpenLevel(World, OverworldMap);

    World = ULevelTransitionHandler::GetCurrentWorld();
    if(ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0))
    {
        Player->GetRootComponent()->SetWorldLocationAndRotation(ReturnPosition, ReturnRotation);
    }
}