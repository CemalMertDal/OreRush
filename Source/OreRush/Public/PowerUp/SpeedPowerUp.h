#pragma once

#include "CoreMinimal.h"
#include "PowerUp/PowerUpBase.h"
#include "SpeedPowerUp.generated.h"

UCLASS()
class ORERUSH_API ASpeedPowerUp : public APowerUpBase
{
	GENERATED_BODY()

protected:
	virtual void ServerApplyTo(AOreRushCharacter* User) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|PowerUp", meta = (ClampMin = "1.0"))
	float SpeedMultiplier = 1.5f;
};
