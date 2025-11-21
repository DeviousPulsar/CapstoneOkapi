// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Attack.h"

FAttackStage::FAttackStage() 
{
    Delay = 0;
    WarningLength = 0;
    DamageLength = 0; 
    Damage = 1;

    Targets = {};
}

double UAttack::Length() const
{
    double Max = 0;
    double StartTime = 0;
    for (const FAttackStage& Atk : AttackStages)
    {
        StartTime += Atk.Delay;
        double Temp = StartTime + Atk.WarningLength + Atk.DamageLength;
        if (Temp > Max)
        {
            Max = Temp;
        }
    }

    if (Max < Cooldown)
    {
        Max = Cooldown;
    }

    return Max;
}

UAttack* UAttack::AsStaticAttack(int x, int y) const
{
    UAttack* Result = NewObject<UAttack>();
    Result->Cooldown = Cooldown;
    Result->bDynamic = false;
    Result->UseTime = UseTime;
    Result->bDefaultParryable = bDefaultParryable;

    Result->WarningEffect = WarningEffect;
	Result->AttackEffect = AttackEffect;
    Result->UnparryableWarningEffect = UnparryableWarningEffect;
	Result->UnparryableAttackEffect = UnparryableAttackEffect;
	

    for (const FAttackStage& Stg : AttackStages)
    {
        FAttackStage NewStg = FAttackStage();
        NewStg.Delay = Stg.Delay;
        NewStg.WarningLength = Stg.WarningLength;
        NewStg.DamageLength = Stg.DamageLength;
        NewStg.Damage = Stg.Damage;

        NewStg.bParriable = Stg.bParriable;

        for (const FGridPosition& Target : Stg.Targets)
        {
            NewStg.Targets.Add(FGridPosition(x + Target.x, y + Target.y));
        }

        NewStg.TargetWarningEffect = Stg.TargetWarningEffect;
        NewStg.TargetAttackEffect = Stg.TargetAttackEffect;

        Result->AttackStages.Add(NewStg);
    }

    return Result;
}

void UAttack::Append(UAttack* Other, float DelayBetweenAttacks)
{
    if (!IsValid(Other))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid UAttack pointer %p to be appaended to UAttack %p"), Other, this);
        return;
    }

    if (Other->AttackStages.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Given Attack %p has no stages to append"), Other);
        return;
    }

    Other->AttackStages[0].Delay += DelayBetweenAttacks;
    AttackStages.Append(Other->AttackStages);
}

void UAttack::Buff(float Buff)
{
    for (FAttackStage& Stg : AttackStages)
    {
        Stg.Damage *= Buff;
    }
}