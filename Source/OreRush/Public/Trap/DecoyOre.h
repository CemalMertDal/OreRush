#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "DecoyOre.generated.h"

UCLASS()
class ORERUSH_API ADecoyOre : public ATrapBase
{
	GENERATED_BODY()

public:
	ADecoyOre();

protected:
	virtual void OnTriggered(AOreRushCharacter* Victim) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap", meta = (ClampMin = "0.0"))
	float StunDuration = 1.f;
};
