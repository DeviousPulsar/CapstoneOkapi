// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelTransitionHandler.h"
#include "PlayerTrackingSubsystem.h"
#include "GameFramework/Character.h"
#include "Internationalization/Regex.h"
#include "Kismet/GameplayStatics.h"

ULevelTransitionHandler::ULevelTransitionHandler(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
    OverworldMap = TEXT("");
    ReturnPosition = FVector(0, 0, 230);
    ReturnRotation = FQuat(1, 0, 0, 0);

    TransitionType = ELevelTransitionType::NONE;
    bLoadScheduled = false;

    if (TransitionScreenClass && !IsValid(TransitionScreen))
    {
        UUserWidget* Widget = CreateWidget(this, TransitionScreenClass);
        if (Widget->IsA(UTransitionScreen::StaticClass()))
        {
            TransitionScreen = (UTransitionScreen*) Widget;
        }
    }
}

void ULevelTransitionHandler::Init()
{
    Super::Init();
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
            TransitionScreen->AddToViewport();
        }
    };
}

void ULevelTransitionHandler::LoadComplete(const float LoadTime, const FString& MapName)
{
    Super::LoadComplete(LoadTime, MapName);

    if(ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
    {
        switch(TransitionType)
        {
            case (ELevelTransitionType::OVERWORLD_TO_OVERWORLD): 
            {
                TArray<AActor*> targets = {};
                UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATransitionTarget::StaticClass(), targets);
                for(AActor* target : targets) 
                {
                    if (IsValid(target) && target->IsA(ATransitionTarget::StaticClass()) && ((ATransitionTarget*) target)->GetTransitionName() == EjectTarget)
                    {
                        Player->GetRootComponent()->SetWorldLocationAndRotation(
                            target->GetActorLocation(), 
                            target->GetActorRotation()
                        );
                        break;
                    }
                }
                break;
            }

            case (ELevelTransitionType::COMBAT_TO_OVERWORLD):
            {
                Player->GetRootComponent()->SetWorldLocationAndRotation(ReturnPosition, ReturnRotation);
                break;
            }

            default:
                break;
        }
    }

    TransitionScreen->OnLoadFinished();
}

void ULevelTransitionHandler::LoadCombatScene(const FName LevelToLoad, const FVector pReturnPosition, const FQuat pReturnRotation)
{
    TransitionType = ELevelTransitionType::OVERWORLD_TO_COMBAT;
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

    TransitionScreen->OnLoadStarted();
    ScheduleLoad(LevelToLoad);

    GetSubsystem<UPlayerTrackingSubsystem>()->LogBattleEntered();
}

void ULevelTransitionHandler::LoadOverworldScene(const FName LevelToLoad, const FString pEjectTarget)
{
    if(bLoadScheduled) { return; }

    TransitionType = ELevelTransitionType::OVERWORLD_TO_OVERWORLD;

    EjectTarget = pEjectTarget;

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

    UWorld* World = GetWorld();

    UE_LOG(
        LogLoad, 
        Log, 
        TEXT("Moving to level %s from level %s."), 
        *LevelToLoad.ToString(), 
        *FName(World->GetMapName()).ToString()
    )

    TransitionScreen->OnLoadStarted();
    ScheduleLoad(LevelToLoad);
}

void ULevelTransitionHandler::ReturnToOverworld()
{
    if(bLoadScheduled) { return; }

    TransitionType = ELevelTransitionType::COMBAT_TO_OVERWORLD;

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

    TransitionScreen->OnLoadStarted();
    ScheduleLoad(OverworldMap);
}

void ULevelTransitionHandler::ReloadCombatScene()
{
    if(bLoadScheduled) { return; }

    TransitionType = ELevelTransitionType::COMBAT_TO_COMBAT;

    UWorld* World = GetWorld();

    FName LevelToLoad = FName(World->GetMapName());
    const FRegexPattern Pattern(TEXT("Level_.*"), ERegexPatternFlags::None);
    FRegexMatcher Regex(Pattern, LevelToLoad.ToString());
    if (Regex.FindNext())
    {
        LevelToLoad = FName(Regex.GetCaptureGroup(0));
    }

    UE_LOG(
        LogLoad, 
        Log, 
        TEXT("Reloading combat scene %s"),  
        *FName(World->GetMapName()).ToString()
    );

    TransitionScreen->OnLoadStarted();
    ScheduleLoad(LevelToLoad);
}

void ULevelTransitionHandler::ScheduleLoad(FName LevelToLoad)
{
    MapScheduledToLoad = LevelToLoad;
    bLoadScheduled = true;
    
    FTimerHandle TimerHandle;
    GetTimerManager().SetTimer(TimerHandle, this, &ULevelTransitionHandler::LoadScheduled, LoadDelay, false);
}

void ULevelTransitionHandler::LoadScheduled()
{
    UGameplayStatics::OpenLevel(GetWorld(), MapScheduledToLoad);
    bLoadScheduled = false;
}