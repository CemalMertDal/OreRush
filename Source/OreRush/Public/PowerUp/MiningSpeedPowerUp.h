#pragma once

#include "CoreMinimal.h"
#include "PowerUp/PowerUpBase.h"
#include "MiningSpeedPowerUp.generated.h"

UCLASS()
class ORERUSH_API AMiningSpeedPowerUp : public APowerUpBase
{
	GENERATED_BODY()

protected:
	virtual void ServerApplyTo(AOreRushCharacter* User) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|PowerUp", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float MiningMultiplier = 0.5f;
};
