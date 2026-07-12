#include "PowerUp/SpeedPowerUp.h"
#include "Character/OreRushCharacter.h"

void ASpeedPowerUp::ServerApplyTo(AOreRushCharacter* User)
{
	if (User)
	{
		User->ServerApplySpeedBuff(SpeedMultiplier, BuffDuration);
	}
}
