
#include "Game/OreRushGameMode.h"
#include "Character/OreRushCharacter.h"
#include "Game/OreRushGameState.h"
#include "Game/DepotZone.h"
#include "Map/MapGenerator.h"
#include "Player/OreRushPlayerController.h"
#include "Player/OreRushPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AOreRushGameMode::AOreRushGameMode()
{
	DefaultPawnClass = AOreRushCharacter::StaticClass();
	PlayerControllerClass = AOreRushPlayerController::StaticClass();
	PlayerStateClass = AOreRushPlayerState::StaticClass();
	GameStateClass = AOreRushGameState::StaticClass();

	bUseSeamlessTravel = true;
}

void AOreRushGameMode::InitGameState()
{
	Super::InitGameState();

	if (AOreRushGameState* GS = Cast<AOreRushGameState>(GameState))
	{
		GS->QuotaTarget = QuotaTarget;

		FMath::RandInit(static_cast<int32>(FPlatformTime::Cycles()));
		GS->MapSeed = FMath::RandRange(1, MAX_int32 - 1);
	}
}

void AOreRushGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	if (AOreRushPlayerState* PS = NewPlayer->GetPlayerState<AOreRushPlayerState>())
	{
		const ETeam Assigned = PickTeamForNewPlayer();
		PS->SetTeam(Assigned);

		UE_LOG(LogTemp, Log, TEXT("[OreRush] Team assigned: %s -> %s"),
			*PS->GetPlayerName(),
			(Assigned == ETeam::Red ? TEXT("Red") : TEXT("Blue")));

		EnsurePlayersAtDepots();
	}
}

void AOreRushGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	if (GetNumPlayers() >= MaxPlayers)
	{
		ErrorMessage = TEXT("Lobi dolu");
	}
}

void AOreRushGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	if (C)
	{
		if (AOreRushPlayerState* PS = C->GetPlayerState<AOreRushPlayerState>())
		{
			if (PS->GetTeam() == ETeam::None)
			{
				PS->SetTeam(PickTeamForNewPlayer());
			}
		}
	}

	EnsurePlayersAtDepots();
}

ETeam AOreRushGameMode::PickTeamForNewPlayer()
{
	return (GetNumPlayers() <= 1) ? ETeam::Red : ETeam::Blue;
}

ADepotZone* AOreRushGameMode::FindDepotForTeam(ETeam Team) const
{
	if (Team == ETeam::None)
	{
		return nullptr;
	}

	for (TActorIterator<ADepotZone> It(GetWorld()); It; ++It)
	{
		if (It->Team == Team)
		{
			return *It;
		}
	}
	return nullptr;
}

AMapGenerator* AOreRushGameMode::FindMapGenerator() const
{
	for (TActorIterator<AMapGenerator> It(GetWorld()); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

AActor* AOreRushGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (Player)
	{
		if (const AOreRushPlayerState* PS = Player->GetPlayerState<AOreRushPlayerState>())
		{
			if (ADepotZone* Depot = FindDepotForTeam(PS->GetTeam()))
			{
				return Depot;
			}
		}
	}

	if (AMapGenerator* MapGen = FindMapGenerator())
	{
		return MapGen;
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void AOreRushGameMode::EnsurePlayersAtDepots()
{
	PlacementAttempts = 0;
	RunPlacementPass();
}

void AOreRushGameMode::RunPlacementPass()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	AMapGenerator* MapGen = FindMapGenerator();
	const bool bHasDepots = FindDepotForTeam(ETeam::Red) != nullptr || FindDepotForTeam(ETeam::Blue) != nullptr;

	if (MapGen == nullptr && !bHasDepots)
	{
		return;
	}

	bool bAllPlaced = true;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC == nullptr || PlacedControllers.Contains(PC))
		{
			continue;
		}

		APawn* Pawn = PC->GetPawn();
		if (Pawn == nullptr)
		{
			bAllPlaced = false;
			continue;
		}

		ETeam Team = ETeam::None;
		if (const AOreRushPlayerState* PS = PC->GetPlayerState<AOreRushPlayerState>())
		{
			Team = PS->GetTeam();
		}
		if (Team == ETeam::None)
		{
			bAllPlaced = false;
			continue;
		}

		ADepotZone* Depot = FindDepotForTeam(Team);
		if (Depot == nullptr)
		{
			bAllPlaced = false;
			continue;
		}

		const FVector Target = Depot->GetActorLocation();
		Pawn->SetActorLocation(Target + FVector(0.f, 0.f, 150.f), false, nullptr, ETeleportType::TeleportPhysics);

		PlacedControllers.Add(PC);
		UE_LOG(LogTemp, Log, TEXT("[OreRush] Placed %s (%s) at %s"),
			*PC->GetName(),
			(Team == ETeam::Red ? TEXT("Red") : TEXT("Blue")),
			*Target.ToString());
	}

	if (!bAllPlaced && PlacementAttempts < 120)
	{
		PlacementAttempts++;
		World->GetTimerManager().SetTimer(PlacementRetryTimer, this, &AOreRushGameMode::RunPlacementPass, 0.25f, false);
	}
}

void AOreRushGameMode::RestartMatch()
{
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("?restart"), false);
	}
}

void AOreRushGameMode::CheckWinCondition()
{
	AOreRushGameState* GS = Cast<AOreRushGameState>(GameState);
	if (!GS || GS->IsMatchEnded())
	{
		return;
	}

	const int32 Quota = GS->QuotaTarget;
	if (Quota <= 0)
	{
		return;
	}

	if (GS->RedScore >= Quota)
	{
		GS->EndMatch(ETeam::Red);
	}
	else if (GS->BlueScore >= Quota)
	{
		GS->EndMatch(ETeam::Blue);
	}
}
