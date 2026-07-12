#include "Components/BuildComponent.h"
#include "Trap/TrapBase.h"
#include "Trap/SnareTrap.h"
#include "Trap/MudTrap.h"
#include "Trap/DecoyOre.h"
#include "Trap/SmokeTrap.h"
#include "Defense/DefenseBase.h"
#include "Defense/Barrier.h"
#include "Defense/Turret.h"
#include "Components/WalletComponent.h"
#include "Player/OreRushPlayerState.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "CollisionQueryParams.h"

UBuildComponent::UBuildComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	auto AddEntry = [this](TSubclassOf<AActor> Cls, int32 Cost)
	{
		FTrapCatalogEntry Entry;
		Entry.PlaceableClass = Cls;
		Entry.Cost = Cost;
		Catalog.Add(Entry);
	};

	AddEntry(ASnareTrap::StaticClass(), 3);
	AddEntry(AMudTrap::StaticClass(), 3);
	AddEntry(ADecoyOre::StaticClass(), 5);
	AddEntry(ASmokeTrap::StaticClass(), 2);
	AddEntry(ABarrier::StaticClass(), 3);
	AddEntry(ATurret::StaticClass(), 5);
}

void UBuildComponent::ServerCycleSelection()
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || OwnerActor->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (Catalog.Num() <= 0)
	{
		return;
	}

	SelectedIndex = (SelectedIndex + 1) % Catalog.Num();

	if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		if (OwnerPawn->IsLocallyControlled())
		{
			OnRep_Selected();
		}
	}
}

FString UBuildComponent::GetSelectedName() const
{
	if (!Catalog.IsValidIndex(SelectedIndex) || !Catalog[SelectedIndex].PlaceableClass)
	{
		return TEXT("None");
	}
	return Catalog[SelectedIndex].PlaceableClass->GetName();
}

int32 UBuildComponent::GetSelectedCost() const
{
	return Catalog.IsValidIndex(SelectedIndex) ? Catalog[SelectedIndex].Cost : 0;
}

int32 UBuildComponent::GetActivePlacedCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AActor>& Placed : PlacedTraps)
	{
		if (Placed.IsValid())
		{
			Count++;
		}
	}
	return Count;
}

void UBuildComponent::OnRep_Selected()
{
	if (GEngine == nullptr || !Catalog.IsValidIndex(SelectedIndex))
	{
		return;
	}

	const UClass* Cls = Catalog[SelectedIndex].PlaceableClass;
	const FString Name = Cls ? Cls->GetName() : TEXT("None");
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan,
		FString::Printf(TEXT("Selected Trap: %s (Cost %d)"), *Name, Catalog[SelectedIndex].Cost));

	OnSelectionChanged.Broadcast();
}

void UBuildComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UBuildComponent, SelectedIndex, COND_OwnerOnly);
}

void UBuildComponent::PrunePlaced()
{
	PlacedTraps.RemoveAll([](const TWeakObjectPtr<AActor>& Placed) { return !Placed.IsValid(); });
}

void UBuildComponent::ServerTryPlace(int32 Index)
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || OwnerActor->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (!Catalog.IsValidIndex(Index))
	{
		return;
	}

	const FTrapCatalogEntry& Entry = Catalog[Index];
	if (!Entry.PlaceableClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const float Now = World->GetTimeSeconds();
	if (Now - LastPlaceTime < PlaceCooldown)
	{
		return;
	}

	PrunePlaced();
	if (PlacedTraps.Num() >= ActiveTrapLimit)
	{
		return;
	}

	UWalletComponent* Wallet = OwnerActor->FindComponentByClass<UWalletComponent>();
	if (Wallet == nullptr || Wallet->GetTotalWorth() < Entry.Cost)
	{
		return;
	}

	const FVector Forward = OwnerActor->GetActorForwardVector();
	const FVector Origin = OwnerActor->GetActorLocation() + Forward * PlaceDistance;

	FVector SpawnLocation = Origin;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);
	if (World->LineTraceSingleByChannel(Hit, Origin + FVector(0.f, 0.f, 100.f), Origin - FVector(0.f, 0.f, 500.f), ECC_Visibility, Params))
	{
		SpawnLocation = Hit.ImpactPoint;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerActor;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Spawned = World->SpawnActor<AActor>(Entry.PlaceableClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (Spawned == nullptr)
	{
		return;
	}

	Wallet->ServerSpendWorth(Entry.Cost);

	ETeam Team = ETeam::None;
	if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor))
	{
		if (const AOreRushPlayerState* PS = OwnerPawn->GetPlayerState<AOreRushPlayerState>())
		{
			Team = PS->GetTeam();
		}
	}

	if (ATrapBase* Trap = Cast<ATrapBase>(Spawned))
	{
		Trap->ServerInit(Team);
	}
	else if (ADefenseBase* Defense = Cast<ADefenseBase>(Spawned))
	{
		Defense->ServerInit(Team);
	}

	PlacedTraps.Add(Spawned);
	LastPlaceTime = Now;
}
