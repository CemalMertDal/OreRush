#pragma once

#include "CoreMinimal.h"
#include "PowerUp/PowerUpBase.h"
#include "ShieldPowerUp.generated.h"

UCLASS()
class ORERUSH_API AShieldPowerUp : public APowerUpBase
{
	GENERATED_BODY()

protected:
	virtual void ServerApplyTo(AOreRushCharacter* User) override;
};
