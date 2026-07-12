#pragma once

#include "CoreMinimal.h"
#include "Trap/TrapBase.h"
#include "SmokeTrap.generated.h"

class ASmokeCloud;

UCLASS()
class ORERUSH_API ASmokeTrap : public ATrapBase
{
	GENERATED_BODY()

protected:
	virtual void OnTriggered(AOreRushCharacter* Victim) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap")
	TSubclassOf<ASmokeCloud> SmokeCloudClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap", meta = (ClampMin = "0.0"))
	float SmokeDuration = 5.f;
};
