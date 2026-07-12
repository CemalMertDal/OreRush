
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Core/OreRushTypes.h"
#include "OreRushPlayerState.generated.h"

UCLASS()
class ORERUSH_API AOreRushPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Ore Rush|Team")
	ETeam GetTeam() const { return Team; }

	void SetTeam(ETeam NewTeam);

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Cosmetic")
	uint8 GetColorIndex() const { return ColorIndex; }

	void SetColorIndex(uint8 NewIndex);

	virtual void CopyProperties(APlayerState* PlayerState) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Team, BlueprintReadOnly, Category = "Ore Rush|Team")
	ETeam Team = ETeam::None;

	UFUNCTION()
	void OnRep_Team();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Team")
	void OnTeamChanged();

	UPROPERTY(ReplicatedUsing = OnRep_Cosmetic, BlueprintReadOnly, Category = "Ore Rush|Cosmetic")
	uint8 ColorIndex = 0;

	UFUNCTION()
	void OnRep_Cosmetic();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Cosmetic")
	void OnCosmeticChanged();
};
