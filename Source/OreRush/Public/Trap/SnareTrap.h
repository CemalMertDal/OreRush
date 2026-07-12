#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "SnareTrap.generated.h"

class AOrePickup;

UCLASS()
class ORERUSH_API ASnareTrap : public ATrapBase
{
	GENERATED_BODY()

protected:
	virtual void OnTriggered(AOreRushCharacter* Victim) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap", meta = (ClampMin = "0.0"))
	float StunDuration = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap")
	TSubclassOf<AOrePickup> PickupClass;
};
