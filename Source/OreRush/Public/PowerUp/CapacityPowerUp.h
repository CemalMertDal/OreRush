#pragma once

#include "CoreMinimal.h"
#include "PowerUp/PowerUpBase.h"
#include "CapacityPowerUp.generated.h"

UCLASS()
class ORERUSH_API ACapacityPowerUp : public APowerUpBase
{
	GENERATED_BODY()

protected:
	virtual void ServerApplyTo(AOreRushCharacter* User) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|PowerUp", meta = (ClampMin = "1"))
	int32 BonusAmount = 10;
};
