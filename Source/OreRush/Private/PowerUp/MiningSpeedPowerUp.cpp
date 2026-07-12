#include "PowerUp/MiningSpeedPowerUp.h"
#include "Character/OreRushCharacter.h"

void AMiningSpeedPowerUp::ServerApplyTo(AOreRushCharacter* User)
{
	if (User)
	{
		User->ServerApplyMiningBuff(MiningMultiplier, BuffDuration);
	}
}
