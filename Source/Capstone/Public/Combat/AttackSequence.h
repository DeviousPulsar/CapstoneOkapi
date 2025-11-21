
#pragma once

#include "Attack.h"
#include "CombatPawn.h"
#include "BattleGrid.h"

#include "CoreMinimal.h"
#include "AttackSequence.generated.h"


USTRUCT(BlueprintType)
struct FAttackTimeoutInfo {
    GENERATED_BODY()

public:
    float UseTime;
    float Cooldown;

    FAttackTimeoutInfo() : UseTime(0), Cooldown(0) {}
    FAttackTimeoutInfo(int32 InUseTime, int32 InCooldown) : UseTime(InUseTime), Cooldown(InCooldown) {}
};


UCLASS(Blueprintable, BlueprintType)
class CAPSTONE_API UAttackSequence : public UObject
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, BlueprintSetter = SetUser, meta=(AllowPrivateAccess=true))
    ACombatPawn* User;

    UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
    TArray<UAttack*> PendingAttacks;

    UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
    int32 CurrentAttackIndex = 0;

public:
    /// The amount of time, in seconds, between attacks in the sequence
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InterAttackDelay;

    /// Sets the current user to the given parameter 
    UFUNCTION(BlueprintCallable)
    void SetUser(ACombatPawn* NewUser);

    /// Clears PendingAttacks and then generates a new sequence of attacks
    /// Returns true if one of more attacks are in the sequence
    UFUNCTION(BlueprintCallable) 
    bool InitalizeSequence(); 

    /// Generates attacks for the sequence
    /// Returns true if one of more attacks are in the sequence
    /// Blueprint Function, cannot be implemented in c++ 
    UFUNCTION(BlueprintImplementableEvent) 
    void GenerateSequence();

    /// Returns The length in seconds that the sequence will play for
    /// Returns 0 if the sequence has not been successfully prepared
    UFUNCTION(BlueprintCallable, BlueprintPure)
    float Length() const;

    /// Returns whether there are additional attacks to execute in the prepared sequence
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool HasNext() const;

    /// Sends the next attack in the prepared sequence to the given BattleGrid for processing
    /// Returns a tuple of the time until the user can move again and the time until the user can attack again
    /// If no next attack exists, returns (0, 0) and does nothing 
    UFUNCTION(BlueprintCallable)
    FAttackTimeoutInfo ExecuteNext(ABattleGrid* Grid);
};