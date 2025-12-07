
#include "Combat/AttackSequence.h"

void UAttackSequence::SetUser(ACombatPawn* NewUser)
{
    if(!IsValid(NewUser))
    {
        return;
    }

    User = NewUser;
}

bool UAttackSequence::InitalizeSequence() 
{
    PendingAttacks.Empty();
    CurrentAttackIndex = 0;

    GenerateSequence();

    return PendingAttacks.Num() > 0;
}

float UAttackSequence::Length() const
{
    float SeqLength = 0;

    for (int32 i = 0; i < PendingAttacks.Num(); ++i)
    {
        SeqLength += PendingAttacks[i]->Length();

        if (i < PendingAttacks.Num() - 1)
        {
            SeqLength += InterAttackDelay;
        }
    }
    
    return SeqLength;
}

bool UAttackSequence::HasNext() const
{
    int32 i = CurrentAttackIndex;
    while (i < PendingAttacks.Num())
    {
        if(IsValid(PendingAttacks[i]) )
        {
            return true;
        }

        i++;
    }

    return false;
}

FAttackTimeoutInfo UAttackSequence::ExecuteNext(ABattleGrid* Grid) 
{
    for (; CurrentAttackIndex < PendingAttacks.Num(); ++CurrentAttackIndex)
    {
        UAttack* Attack = PendingAttacks[CurrentAttackIndex];
        if(IsValid(Attack))
        {
            Grid->ExecuteAttack(Attack);
            FAttackTimeoutInfo info(Attack->UseTime, Attack->Cooldown);
            CurrentAttackIndex++;
            return info;
        }
    }

    return FAttackTimeoutInfo(0, 0);
}