#include "PowerUp/CapacityPowerUp.h"
#include "Character/OreRushCharacter.h"
#include "Components/WalletComponent.h"

void ACapacityPowerUp::ServerApplyTo(AOreRushCharacter* User)
{
	if (User == nullptr)
	{
		return;
	}

	if (UWalletComponent* Wallet = User->GetWallet())
	{
		Wallet->ServerApplyCapacityBonus(BonusAmount, BuffDuration);
	}
}
