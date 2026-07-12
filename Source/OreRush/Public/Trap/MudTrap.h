#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "MudTrap.generated.h"

UCLASS()
class ORERUSH_API AMudTrap : public ATrapBase
{
	GENERATED_BODY()

protected:
	virtual void OnTriggered(AOreRushCharacter* Victim) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SlowMultiplier = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap", meta = (ClampMin = "0.0"))
	float SlowDuration = 3.f;
};
