
#pragma once

#include "CoreMinimal.h"
#include "OreRushTypes.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None UMETA(DisplayName = "None"),
	Red  UMETA(DisplayName = "Red"),
	Blue UMETA(DisplayName = "Blue")
};

UENUM(BlueprintType)
enum class EOreType : uint8
{
	None    = 0 UMETA(DisplayName = "None"),
	Iron    = 1 UMETA(DisplayName = "Iron"),
	Gold    = 2 UMETA(DisplayName = "Gold"),
	Diamond = 5 UMETA(DisplayName = "Diamond")
};

FORCEINLINE int32 OreWorth(EOreType Type)
{
	return static_cast<int32>(Type);
}

UENUM(BlueprintType)
enum class EOreRushBuff : uint8
{
	Speed   UMETA(DisplayName = "Speed"),
	Mining  UMETA(DisplayName = "Mining"),
	Shield  UMETA(DisplayName = "Shield")
};
