
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/OreRushTypes.h"
#include "OreRushGameState.generated.h"

UCLASS()
class ORERUSH_API AOreRushGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(ReplicatedUsing = OnRep_Scores, BlueprintReadOnly, Category = "Ore Rush|Score")
	int32 RedScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Scores, BlueprintReadOnly, Category = "Ore Rush|Score")
	int32 BlueScore = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Score")
	int32 QuotaTarget = 50;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Map")
	int32 MapSeed = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchEnded, BlueprintReadOnly, Category = "Ore Rush|Match")
	bool bMatchEnded = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Match")
	ETeam WinningTeam = ETeam::None;

	void AddScore(ETeam Team, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Score")
	int32 GetScore(ETeam Team) const;

	int32 RemoveScore(ETeam Team, int32 Amount);

	void EndMatch(ETeam InWinningTeam);

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Match")
	bool IsMatchEnded() const { return bMatchEnded; }

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Match")
	ETeam GetWinningTeam() const { return WinningTeam; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Scores();

	UFUNCTION()
	void OnRep_MatchEnded();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Score")
	void OnScoresChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Match")
	void OnMatchEnded();
};
