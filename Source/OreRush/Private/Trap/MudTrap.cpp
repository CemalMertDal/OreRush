#include "Trap/MudTrap.h"
#include "Character/OreRushCharacter.h"

void AMudTrap::OnTriggered(AOreRushCharacter* Victim)
{
	if (Victim)
	{
		Victim->ServerApplySlow(SlowMultiplier, SlowDuration);
	}
}
