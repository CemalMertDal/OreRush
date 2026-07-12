#include "Trap/SnareTrap.h"
#include "Character/OreRushCharacter.h"
#include "Components/WalletComponent.h"
#include "Pickup/OrePickup.h"
#include "Engine/World.h"

void ASnareTrap::OnTriggered(AOreRushCharacter* Victim)
{
	if (Victim == nullptr)
	{
		return;
	}

	Victim->ServerApplyStun(StunDuration);

	UWalletComponent* Wallet = Victim->GetWallet();
	if (Wallet == nullptr)
	{
		return;
	}

	int32 Iron = 0;
	int32 Gold = 0;
	int32 Diamond = 0;
	Wallet->ServerTakeAll(Iron, Gold, Diamond);

	if (Iron + Gold + Diamond <= 0)
	{
		return;
	}

	TSubclassOf<AOrePickup> Cls = PickupClass;
	if (!Cls)
	{
		Cls = AOrePickup::StaticClass();
	}
	const FVector SpawnLocation = Victim->GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AOrePickup* Pickup = GetWorld()->SpawnActor<AOrePickup>(Cls, SpawnLocation, FRotator::ZeroRotator, SpawnParams))
	{
		Pickup->ServerInit(Iron, Gold, Diamond);
	}
}
