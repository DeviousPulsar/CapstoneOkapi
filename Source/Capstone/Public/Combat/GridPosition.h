
#pragma once

#include "CoreMinimal.h"
#include "GridPosition.generated.h"

USTRUCT(BlueprintType)
struct FGridPosition
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly);
	int x;

    UPROPERTY(EditAnywhere, BlueprintReadOnly);
	int y;

    FGridPosition() : x(0), y(0) {}
    FGridPosition(int32 InX, int32 InY) : x(InX), y(InY) {}
};