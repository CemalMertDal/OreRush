#include "Game/DepotZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/OreRushCharacter.h"
#include "Components/WalletComponent.h"
#include "Player/OreRushPlayerState.h"
#include "Player/OreRushPlayerController.h"
#include "Game/OreRushGameState.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Defense/DefenseBase.h"

ADepotZone::ADepotZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->InitBoxExtent(FVector(150.f, 150.f, 100.f));
	OverlapBox->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(OverlapBox);

	InteractBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractBox"));
	InteractBox->SetupAttachment(OverlapBox);
	InteractBox->InitBoxExtent(FVector(150.f, 150.f, 100.f));
	InteractBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractBox->SetCollisionObjectType(ECC_WorldStatic);
	InteractBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(OverlapBox);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADepotZone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ADepotZone::OnOverlapBegin);
	}
}

void ADepotZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
								bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	AOreRushCharacter* Character = Cast<AOreRushCharacter>(OtherActor);
	if (!Character)
	{
		return;
	}

	AOreRushPlayerState* PS = Cast<AOreRushPlayerState>(Character->GetPlayerState());
	if (!PS)
	{
		return;
	}

	if (bBeingRaided && Team != ETeam::None && PS->GetTeam() == Team)
	{
		EndRaid(true);
	}

	if (Team == ETeam::None)
	{
		return;
	}

	if (PS->GetTeam() != Team)
	{
		return;
	}

	UWalletComponent* Wallet = Character->GetWallet();
	if (!Wallet)
	{
		return;
	}

	const int32 TotalUnits = Wallet->GetTotalUnits();
	if (TotalUnits <= 0)
	{
		FString Msg = FString::Printf(TEXT("[%s] Depolanacak cevher yok! Cüzdan bos."), *Character->GetName());
		UE_LOG(LogTemp, Log, TEXT("%s"), *Msg);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Msg);
		}
		return;
	}

	const int32 ClearedWorth = Wallet->ServerClear();
	if (ClearedWorth > 0)
	{
		if (AOreRushGameState* GS = GetWorld()->GetGameState<AOreRushGameState>())
		{
			GS->AddScore(Team, ClearedWorth);
		}
		MulticastOnDepositFX(ClearedWorth);

		FString Msg = FString::Printf(TEXT("[%s] %d adet cevher (%d degerinde) basariyla depolandi!"),
			*Character->GetName(), TotalUnits, ClearedWorth);
		
		UE_LOG(LogTemp, Display, TEXT("%s"), *Msg);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Msg);
		}
	}
}

void ADepotZone::MulticastOnDepositFX_Implementation(int32 Amount)
{
	OnDepositFX(Amount);
}

bool ADepotZone::CanInteract(AOreRushCharacter* User) const
{
	if (User == nullptr || Team == ETeam::None)
	{
		return false;
	}

	const AOreRushPlayerState* PS = User->GetPlayerState<AOreRushPlayerState>();
	if (PS == nullptr || PS->GetTeam() == ETeam::None || PS->GetTeam() == Team)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (World == nullptr || World->GetTimeSeconds() < RaidClosedUntil)
	{
		return false;
	}

	const AOreRushGameState* GS = World->GetGameState<AOreRushGameState>();
	if (GS == nullptr || GS->IsMatchEnded() || GS->GetScore(Team) <= 0)
	{
		return false;
	}

	return true;
}

void ADepotZone::ServerStartInteract(AOreRushCharacter* User)
{
	if (!HasAuthority() || !CanInteract(User))
	{
		return;
	}

	if (IsOwnerPresent())
	{
		CurrentRaider = User;
		StolenWorthThisRaid = 0;
		EndRaid(true);
		return;
	}

	CurrentRaider = User;
	StolenWorthThisRaid = 0;
	bBeingRaided = true;
	OnRep_BeingRaided();

	SendAlarmToOwner(TEXT("Deponuza baskin yapiliyor!"));

	GetWorldTimerManager().SetTimer(RaidTimerHandle, this, &ADepotZone::RaidTick, RaidTickInterval, true, RaidTickInterval);
}

void ADepotZone::ServerStopInteract(AOreRushCharacter* User)
{
	if (CurrentRaider.Get() == User)
	{
		EndRaid(false);
	}
}

void ADepotZone::RaidTick()
{
	if (!HasAuthority())
	{
		return;
	}

	AOreRushCharacter* Raider = CurrentRaider.Get();
	AOreRushGameState* GS = GetWorld() ? GetWorld()->GetGameState<AOreRushGameState>() : nullptr;
	if (Raider == nullptr || GS == nullptr || GS->IsMatchEnded() || GS->GetScore(Team) <= 0)
	{
		EndRaid(false);
		return;
	}

	UWalletComponent* Wallet = Raider->GetWallet();
	if (Wallet == nullptr || Wallet->IsFull())
	{
		return;
	}

	const int32 EffectiveSteal = FMath::Max(1, StealPerTick - CountTeamDefensesNear());
	const int32 Want = FMath::Min(EffectiveSteal, GS->GetScore(Team));
	int32 Got = 0;
	for (int32 i = 0; i < Want; ++i)
	{
		if (Wallet->IsFull() || !Wallet->ServerAddOre(EOreType::Iron))
		{
			break;
		}
		Got++;
	}

	if (Got > 0)
	{
		GS->RemoveScore(Team, Got);
		StolenWorthThisRaid += Got;
	}
}

void ADepotZone::EndRaid(bool bBusted)
{
	if (!HasAuthority())
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(RaidTimerHandle);

	AOreRushCharacter* Raider = CurrentRaider.Get();

	if (bBusted && Raider)
	{
		if (UWalletComponent* Wallet = Raider->GetWallet())
		{
			Wallet->ServerSpendWorth(StolenWorthThisRaid);
		}
		if (AOreRushGameState* GS = GetWorld()->GetGameState<AOreRushGameState>())
		{
			GS->AddScore(Team, StolenWorthThisRaid);
		}

		ETeam RaiderTeam = ETeam::None;
		if (const AOreRushPlayerState* PS = Raider->GetPlayerState<AOreRushPlayerState>())
		{
			RaiderTeam = PS->GetTeam();
		}
		if (ADepotZone* HomeDepot = FindDepotForTeam(RaiderTeam))
		{
			const FVector Tp = HomeDepot->GetActorLocation() + FVector(0.f, 0.f, 120.f);
			Raider->SetActorLocation(Tp, false, nullptr, ETeleportType::TeleportPhysics);
		}

		SendAlarmToOwner(TEXT("Baskinci yakalandi! Depo korundu."));
		if (AOreRushPlayerController* PC = Cast<AOreRushPlayerController>(Raider->GetController()))
		{
			PC->ClientShowAlarm(TEXT("BUSTED! Calinanlar geri alindi, ussune isinlandirildin."));
		}

		RaidClosedUntil = GetWorld()->GetTimeSeconds() + RaidGrace;
	}

	if (Raider)
	{
		Raider->NotifyInteractFinished(this);
	}

	CurrentRaider = nullptr;
	StolenWorthThisRaid = 0;
	bBeingRaided = false;
	OnRep_BeingRaided();
}

bool ADepotZone::IsOwnerPresent() const
{
	if (OverlapBox == nullptr || Team == ETeam::None)
	{
		return false;
	}

	TArray<AActor*> Overlapping;
	OverlapBox->GetOverlappingActors(Overlapping, AOreRushCharacter::StaticClass());
	for (AActor* A : Overlapping)
	{
		const AOreRushCharacter* C = Cast<AOreRushCharacter>(A);
		if (C == nullptr)
		{
			continue;
		}
		const AOreRushPlayerState* PS = C->GetPlayerState<AOreRushPlayerState>();
		if (PS && PS->GetTeam() == Team)
		{
			return true;
		}
	}
	return false;
}

int32 ADepotZone::CountTeamDefensesNear() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr || Team == ETeam::None)
	{
		return 0;
	}

	const float RadiusSq = ReinforceRadius * ReinforceRadius;
	const FVector Origin = GetActorLocation();
	int32 Count = 0;

	for (TActorIterator<ADefenseBase> It(World); It; ++It)
	{
		if (It->OwnerTeam == Team && FVector::DistSquared(It->GetActorLocation(), Origin) <= RadiusSq)
		{
			Count++;
		}
	}
	return Count;
}

void ADepotZone::SendAlarmToOwner(const FString& Message) const
{
	const AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
	if (GS == nullptr)
	{
		return;
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		const AOreRushPlayerState* OPS = Cast<AOreRushPlayerState>(PS);
		if (OPS && OPS->GetTeam() == Team)
		{
			if (AOreRushPlayerController* PC = Cast<AOreRushPlayerController>(OPS->GetPlayerController()))
			{
				PC->ClientShowAlarm(Message);
			}
		}
	}
}

ADepotZone* ADepotZone::FindDepotForTeam(ETeam InTeam) const
{
	if (InTeam == ETeam::None)
	{
		return nullptr;
	}

	for (TActorIterator<ADepotZone> It(GetWorld()); It; ++It)
	{
		if (*It != this && It->Team == InTeam)
		{
			return *It;
		}
	}
	return nullptr;
}

void ADepotZone::OnRep_BeingRaided()
{
	OnRaidStateChanged(bBeingRaided);
}

void ADepotZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADepotZone, bBeingRaided);
}
