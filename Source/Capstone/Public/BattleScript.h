// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BattleScript.generated.h"


UCLASS()
class CAPSTONE_API ABattleScript : public AActor
{
	GENERATED_BODY()
	//UPROPERTY(EditAnywhere) std::string FileName;

	//std::ifstream FileReadStream;
	//TArray<>
	
public:	
	// Sets default values for this actor's properties
	ABattleScript();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
