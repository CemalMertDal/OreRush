#include "PowerUp/ShieldPowerUp.h"
#include "Character/OreRushCharacter.h"

void AShieldPowerUp::ServerApplyTo(AOreRushCharacter* User)
{
	if (User)
	{
		User->ServerApplyShield(BuffDuration);
	}
}
