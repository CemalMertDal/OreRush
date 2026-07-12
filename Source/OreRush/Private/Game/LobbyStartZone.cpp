#include "Game/LobbyStartZone.h"
#include "Components/BoxComponent.h"
#include "Character/OreRushCharacter.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ALobbyStartZone::ALobbyStartZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	Zone->InitBoxExtent(FVector(200.f, 200.f, 100.f));
	Zone->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Zone);

	CountdownSeconds = StartSeconds;
}

void ALobbyStartZone::BeginPlay()
{
	Super::BeginPlay();

	CountdownSeconds = StartSeconds;

	if (HasAuthority())
	{
		Zone->OnComponentBeginOverlap.AddDynamic(this, &ALobbyStartZone::OnZoneBegin);
		Zone->OnComponentEndOverlap.AddDynamic(this, &ALobbyStartZone::OnZoneEnd);
	}
}

void ALobbyStartZone::OnZoneBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AOreRushCharacter* Char = Cast<AOreRushCharacter>(OtherActor))
	{
		Inside.AddUnique(Char);
		Recompute();
	}
}

void ALobbyStartZone::OnZoneEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AOreRushCharacter* Char = Cast<AOreRushCharacter>(OtherActor))
	{
		Inside.Remove(Char);
		Recompute();
	}
}

void ALobbyStartZone::Recompute()
{
	Inside.RemoveAll([](const TWeakObjectPtr<AOreRushCharacter>& C) { return !C.IsValid(); });

	if (Inside.Num() >= RequiredPlayers)
	{
		StartCountdown();
	}
	else
	{
		CancelCountdown();
	}
}

void ALobbyStartZone::StartCountdown()
{
	if (bCounting)
	{
		return;
	}

	bCounting = true;
	CountdownSeconds = StartSeconds;
	OnRep_Counting();
	OnRep_Countdown();

	GetWorldTimerManager().SetTimer(CountdownTimer, this, &ALobbyStartZone::Tick1s, 1.f, true, 1.f);
}

void ALobbyStartZone::CancelCountdown()
{
	GetWorldTimerManager().ClearTimer(CountdownTimer);

	if (!bCounting && CountdownSeconds == StartSeconds)
	{
		return;
	}

	bCounting = false;
	CountdownSeconds = StartSeconds;
	OnRep_Counting();
	OnRep_Countdown();
}

void ALobbyStartZone::Tick1s()
{
	if (!HasAuthority())
	{
		return;
	}

	Inside.RemoveAll([](const TWeakObjectPtr<AOreRushCharacter>& C) { return !C.IsValid(); });
	if (Inside.Num() < RequiredPlayers)
	{
		CancelCountdown();
		return;
	}

	CountdownSeconds--;
	OnRep_Countdown();

	if (CountdownSeconds <= 0)
	{
		GetWorldTimerManager().ClearTimer(CountdownTimer);
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(GameMapPath);
		}
	}
}

void ALobbyStartZone::OnRep_Countdown()
{
	OnCountdownChanged(CountdownSeconds);
}

void ALobbyStartZone::OnRep_Counting()
{
	OnCountingChanged(bCounting);
}

void ALobbyStartZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyStartZone, CountdownSeconds);
	DOREPLIFETIME(ALobbyStartZone, bCounting);
}
