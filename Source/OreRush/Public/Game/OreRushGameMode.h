
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/OreRushTypes.h"
#include "OreRushGameMode.generated.h"

class ADepotZone;
class AMapGenerator;

UCLASS()
class ORERUSH_API AOreRushGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOreRushGameMode();

	virtual void InitGameState() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	void CheckWinCondition();

	void EnsurePlayersAtDepots();

	void RestartMatch();

protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	UPROPERTY(EditDefaultsOnly, Category = "Ore Rush|Match")
	int32 QuotaTarget = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Ore Rush|Match", meta = (ClampMin = "1"))
	int32 MaxPlayers = 2;

	ETeam PickTeamForNewPlayer();

	ADepotZone* FindDepotForTeam(ETeam Team) const;

	AMapGenerator* FindMapGenerator() const;

private:
	void RunPlacementPass();

	TSet<TWeakObjectPtr<AController>> PlacedControllers;
	FTimerHandle PlacementRetryTimer;
	int32 PlacementAttempts = 0;
};
