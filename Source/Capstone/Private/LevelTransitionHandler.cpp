// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelTransitionHandler.h"
#include "CapstoneSaveGame.h"
#include "GameFramework/Character.h"
#include "Internationalization/Regex.h"
#include "Kismet/GameplayStatics.h"

void ULevelTransitionHandler::Init()
{
    Super::Init();
}

ULevelTransitionHandler::ULevelTransitionHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    OverworldMap = TEXT("");
    ReturnPosition = FVector(0, 0, 230);
    ReturnRotation = FQuat(1, 0, 0, 0);

    TransitionType = ELevelTransitionType::NONE;

}

void ULevelTransitionHandler::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
    Super::OnWorldChanged(OldWorld, NewWorld);

    if (TransitionScreenClass)
    {
        if (!IsValid(TransitionScreen))
        {
            UUserWidget* Widget = CreateWidget(this, TransitionScreenClass);
            if (Widget->IsA(UTransitionScreen::StaticClass()))
            {
                TransitionScreen = (UTransitionScreen*) Widget;
            }
        }

        if (!TransitionScreen->IsInViewport())
        {
            TransitionScreen->AddToViewport(0x7FFFFFFF);
        }
    };
}

void ULevelTransitionHandler::LoadComplete(const float LoadTime, const FString& MapName)
{
    Super::LoadComplete(LoadTime, MapName);

    if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        FTransform Dest = GetRespawnTransform();
        Player->GetRootComponent()->SetWorldLocationAndRotation(Dest.GetLocation(), Dest.GetRotation());
    }

    if (IsValid(TransitionScreen))
    {
        TransitionScreen->OnLoadFinished();
    }
}

void ULevelTransitionHandler::LoadScene(const FName LevelToLoad, bool bToSchedule)
{
    if (bLoadScheduled) { return; }

    UWorld* World = GetWorld();
    FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(World, true);

    switch (TransitionType)
    {
        case(ELevelTransitionType::TRANSFORM):
            UE_LOG(
                LogLoad,
                Log,
                TEXT("Moving to Level %s from level %s. Player actor will return with position %s and facing %s"),
                *LevelToLoad.ToString(),
                *CurrentLevel,
                *ReturnPosition.ToString(),
                *ReturnRotation.Rotator().ToString()
            ); 
            break;
        
        case(ELevelTransitionType::TARGET):
            UE_LOG(
                LogLoad,
                Log,
                TEXT("Moving to Level %s from level %s. Player actor spawn onto ATransitionTarget with transition name: %s"),
                *LevelToLoad.ToString(),
                *CurrentLevel,
                *EjectTarget
            );
            break;

        default:
            UE_LOG(
                LogLoad,
                Log,
                TEXT("Moving to Level %s from level %s."),
                *LevelToLoad.ToString(),
                *CurrentLevel
            );
            break;
    }

    if(bToSchedule)
    {
        if (IsValid(TransitionScreen))
        {
            if (!TransitionScreen->IsInViewport())
            {
                TransitionScreen->RemoveFromParent();
            }
            TransitionScreen->AddToViewport(0x7FFFFFFF);
            TransitionScreen->OnLoadStarted();
        }

        ScheduleLoad(LevelToLoad);
    }
    else
    {
        UGameplayStatics::OpenLevel(World, LevelToLoad);
    }
}

void ULevelTransitionHandler::LoadCombatScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation)
{
    UWorld* World = GetWorld();
    OverworldMap = FName(*UGameplayStatics::GetCurrentLevelName(World, true));

    TransitionType = ELevelTransitionType::TRANSFORM;
    ReturnPosition = pReturnPosition;
    ReturnRotation = pReturnRotation;

    LoadScene(LevelToLoad);
}

void ULevelTransitionHandler::LoadCombatSceneWithReturnTarget(const FName LevelToLoad, const FString pEjectTarget)
{
    UWorld* World = GetWorld();
    OverworldMap = FName(*UGameplayStatics::GetCurrentLevelName(World, true));
    
    TransitionType = ELevelTransitionType::TARGET;
    EjectTarget = pEjectTarget;

    LoadScene(LevelToLoad);
}

void ULevelTransitionHandler::LoadOverworldScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation)
{
    TransitionType = ELevelTransitionType::TRANSFORM;
    ReturnPosition = pReturnPosition;
    ReturnRotation = pReturnRotation;

    LoadScene(LevelToLoad);
}

void ULevelTransitionHandler::LoadOverworldSceneWithSpawnTarget(const FName LevelToLoad, const FString pEjectTarget)
{
    TransitionType = ELevelTransitionType::TARGET;
    EjectTarget = pEjectTarget;

    LoadScene(LevelToLoad);
}

void ULevelTransitionHandler::ReturnToOverworld()
{
    if (!OverworldMap.IsNone())
    {
        LoadScene(OverworldMap, false);
    }
}

void ULevelTransitionHandler::ReloadCombatScene()
{
    UWorld* World = GetWorld();
    FName LevelToLoad = FName(*UGameplayStatics::GetCurrentLevelName(World, true));

    LoadScene(LevelToLoad, false);
}

void ULevelTransitionHandler::ScheduleLoad(FName LevelToLoad)
{
    MapScheduledToLoad = LevelToLoad;
    bLoadScheduled = true;

    GetTimerManager().SetTimer(TimerHandle, this, &ULevelTransitionHandler::LoadScheduled, LoadDelay, false);
}

void ULevelTransitionHandler::LoadScheduled()
{
    UGameplayStatics::OpenLevel(GetWorld(), MapScheduledToLoad);
    bLoadScheduled = false;
}

FTransform ULevelTransitionHandler::GetRespawnTransform() const
{
    switch (TransitionType)
    {
        case (ELevelTransitionType::TARGET):
        {
            TArray<AActor*> targets = {};
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATransitionTarget::StaticClass(), targets);
            for (AActor* target : targets)
            {
                if (IsValid(target) && target->IsA(ATransitionTarget::StaticClass()) && 
                    ((ATransitionTarget*)target)->GetTransitionName() == EjectTarget)
                {
                    return FTransform(target->GetActorRotation(), target->GetActorLocation(),FVector(1,1,1));
                }
            }
            break;
        }

        case (ELevelTransitionType::TRANSFORM):
        {
            return FTransform(ReturnRotation, ReturnPosition, FVector(1,1,1));
        }

        default:
            break;
    }

    return FTransform();
}