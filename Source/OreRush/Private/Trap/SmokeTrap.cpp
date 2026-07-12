#include "Trap/SmokeTrap.h"
#include "Trap/SmokeCloud.h"
#include "Engine/World.h"

void ASmokeTrap::OnTriggered(AOreRushCharacter* Victim)
{
	if (!HasAuthority())
	{
		return;
	}

	TSubclassOf<ASmokeCloud> Cls = SmokeCloudClass;
	if (!Cls)
	{
		Cls = ASmokeCloud::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ASmokeCloud* Cloud = GetWorld()->SpawnActor<ASmokeCloud>(Cls, GetActorLocation(), FRotator::ZeroRotator, SpawnParams))
	{
		Cloud->ServerInit(SmokeDuration);
	}
}
