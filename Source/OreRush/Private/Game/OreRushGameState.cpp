
#include "Game/OreRushGameState.h"
#include "Game/OreRushGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

void AOreRushGameState::OnRep_Scores()
{
	OnScoresChanged();
}

void AOreRushGameState::AddScore(ETeam Team, int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || bMatchEnded)
	{
		return;
	}

	if (Team == ETeam::Red)
	{
		RedScore += Amount;
	}
	else if (Team == ETeam::Blue)
	{
		BlueScore += Amount;
	}
	else
	{
		return;
	}

	OnRep_Scores();

	if (AOreRushGameMode* GM = GetWorld()->GetAuthGameMode<AOreRushGameMode>())
	{
		GM->CheckWinCondition();
	}
}

int32 AOreRushGameState::GetScore(ETeam Team) const
{
	if (Team == ETeam::Red)
	{
		return RedScore;
	}
	if (Team == ETeam::Blue)
	{
		return BlueScore;
	}
	return 0;
}

int32 AOreRushGameState::RemoveScore(ETeam Team, int32 Amount)
{
	if (!HasAuthority() || Amount <= 0 || bMatchEnded)
	{
		return 0;
	}

	int32* Score = nullptr;
	if (Team == ETeam::Red)
	{
		Score = &RedScore;
	}
	else if (Team == ETeam::Blue)
	{
		Score = &BlueScore;
	}
	if (Score == nullptr)
	{
		return 0;
	}

	const int32 Removed = FMath::Min(Amount, *Score);
	if (Removed > 0)
	{
		*Score -= Removed;
		OnRep_Scores();
	}
	return Removed;
}

void AOreRushGameState::EndMatch(ETeam InWinningTeam)
{
	if (!HasAuthority() || bMatchEnded)
	{
		return;
	}

	WinningTeam = InWinningTeam;
	bMatchEnded = true;
	OnRep_MatchEnded();
}

void AOreRushGameState::OnRep_MatchEnded()
{
	OnMatchEnded();

	if (GEngine)
	{
		const TCHAR* TeamName = (WinningTeam == ETeam::Red)
			? TEXT("Kirmizi")
			: (WinningTeam == ETeam::Blue ? TEXT("Mavi") : TEXT("None"));

		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Cyan,
			FString::Printf(TEXT("MAC BITTI! Kazanan: %s takimi"), TeamName));
	}
}

void AOreRushGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreRushGameState, RedScore);
	DOREPLIFETIME(AOreRushGameState, BlueScore);
	DOREPLIFETIME(AOreRushGameState, QuotaTarget);
	DOREPLIFETIME(AOreRushGameState, MapSeed);
	DOREPLIFETIME(AOreRushGameState, WinningTeam);
	DOREPLIFETIME(AOreRushGameState, bMatchEnded);
}
