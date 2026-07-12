
#include "Ore/OreVein.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Character/OreRushCharacter.h"
#include "Components/WalletComponent.h"
#include "Kismet/GameplayStatics.h"

AOreVein::AOreVein()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->InitBoxExtent(FVector(50.f, 50.f, 50.f));
	InteractBox->SetCollisionProfileName(TEXT("BlockAll"));
	SetRootComponent(InteractBox);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(InteractBox);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MineTimes.Add(EOreType::Iron, 1.0f);
	MineTimes.Add(EOreType::Gold, 2.5f);
	MineTimes.Add(EOreType::Diamond, 5.0f);
}

float AOreVein::GetEffectiveMineTime() const
{
	if (const float* Found = MineTimes.Find(OreType))
	{
		return FMath::Max(0.05f, *Found);
	}
	return FMath::Max(0.05f, MineTime);
}

void AOreVein::BeginPlay()
{
	Super::BeginPlay();
	ApplyMeshForType();
}

void AOreVein::OnRep_OreType()
{
	ApplyMeshForType();
}

void AOreVein::ApplyMeshForType()
{
	if (Mesh == nullptr)
	{
		return;
	}

	if (const TObjectPtr<UStaticMesh>* Found = OreMeshes.Find(OreType))
	{
		if (*Found)
		{
			Mesh->SetStaticMesh(*Found);
		}
	}
}

bool AOreVein::CanInteract(AOreRushCharacter* User) const
{
	return User != nullptr && CanBeMined();
}

void AOreVein::ServerStartInteract(AOreRushCharacter* User)
{
	if (!HasAuthority() || User == nullptr || !CanBeMined())
	{
		return;
	}

	CurrentMiner = User;

	const float Interval = FMath::Max(0.05f, GetEffectiveMineTime() * User->GetMiningSpeedMultiplier());
	GetWorldTimerManager().SetTimer(MineTimerHandle, this, &AOreVein::MineTick, Interval, true, Interval);
}

void AOreVein::ServerStopInteract(AOreRushCharacter* User)
{
	if (CurrentMiner.Get() == User)
	{
		GetWorldTimerManager().ClearTimer(MineTimerHandle);
		CurrentMiner = nullptr;
	}
}

void AOreVein::MineTick()
{
	if (!HasAuthority())
	{
		return;
	}

	AOreRushCharacter* Miner = CurrentMiner.Get();
	if (Miner == nullptr || !CanBeMined())
	{
		GetWorldTimerManager().ClearTimer(MineTimerHandle);
		if (Miner)
		{
			Miner->NotifyInteractFinished(this);
		}
		CurrentMiner = nullptr;
		return;
	}

	UWalletComponent* Wallet = Miner->GetWallet();
	if (Wallet == nullptr || Wallet->IsFull())
	{
		return;
	}

	const EOreType Extracted = ServerExtractOne();
	if (Extracted != EOreType::None)
	{
		Wallet->ServerAddOre(Extracted);
	}
}

EOreType AOreVein::ServerExtractOne()
{
	if (!HasAuthority() || bDepleted)
	{
		return EOreType::None;
	}

	if (bUnlimited)
	{
		MulticastMineHit();
		return OreType;
	}

	if (RemainingUnits <= 0)
	{
		return EOreType::None;
	}

	RemainingUnits--;
	MulticastMineHit();
	OnRep_RemainingUnits(); // sunucuda da güncelle (depletion FX vb.)

	if (RemainingUnits <= 0)
	{
		Deplete();
	}

	return OreType;
}

void AOreVein::Deplete()
{
	bDepleted = true;
	SetActorEnableCollision(false);

	GetWorldTimerManager().ClearTimer(MineTimerHandle);
	if (AOreRushCharacter* Miner = CurrentMiner.Get())
	{
		Miner->NotifyInteractFinished(this);
	}
	CurrentMiner = nullptr;

	SetLifeSpan(0.2f);
}

void AOreVein::OnRep_RemainingUnits()
{
	if (!bUnlimited && RemainingUnits <= 0)
	{
		OnDepletedFX();
	}
}

void AOreVein::MulticastMineHit_Implementation()
{
	OnMineHitFX();
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
}

void AOreVein::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreVein, OreType);
	DOREPLIFETIME(AOreVein, bUnlimited);
	DOREPLIFETIME(AOreVein, RemainingUnits);
}
