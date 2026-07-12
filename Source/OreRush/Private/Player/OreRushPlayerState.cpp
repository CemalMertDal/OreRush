
#include "Player/OreRushPlayerState.h"
#include "Net/UnrealNetwork.h"

void AOreRushPlayerState::SetTeam(ETeam NewTeam)
{
	if (!HasAuthority() || Team == NewTeam)
	{
		return;
	}

	Team = NewTeam;
	OnRep_Team(); // RepNotify client'larda; sunucuda elle tetikle.
}

void AOreRushPlayerState::OnRep_Team()
{
	OnTeamChanged();
}

void AOreRushPlayerState::SetColorIndex(uint8 NewIndex)
{
	if (!HasAuthority() || ColorIndex == NewIndex)
	{
		return;
	}

	ColorIndex = NewIndex;
	OnRep_Cosmetic();
}

void AOreRushPlayerState::OnRep_Cosmetic()
{
	OnCosmeticChanged();
}

void AOreRushPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AOreRushPlayerState* PS = Cast<AOreRushPlayerState>(PlayerState))
	{
		PS->Team = Team;
		PS->ColorIndex = ColorIndex;
	}
}

void AOreRushPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreRushPlayerState, Team);
	DOREPLIFETIME(AOreRushPlayerState, ColorIndex);
}
