
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "Interaction/OreRushInteractable.h"
#include "DepotZone.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AOreRushCharacter;

UCLASS()
class ORERUSH_API ADepotZone : public AActor, public IOreRushInteractable
{
	GENERATED_BODY()

public:
	ADepotZone();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	ETeam Team = ETeam::None;

	virtual bool CanInteract(AOreRushCharacter* User) const override;
	virtual void ServerStartInteract(AOreRushCharacter* User) override;
	virtual void ServerStopInteract(AOreRushCharacter* User) override;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Depot")
	bool IsBeingRaided() const { return bBeingRaided; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "1"))
	int32 StealPerTick = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "0.05"))
	float RaidTickInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "0.0"))
	float RaidGrace = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid", meta = (ClampMin = "0.0"))
	float ReinforceRadius = 1500.f;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UBoxComponent> OverlapBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UBoxComponent> InteractBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Depot")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(ReplicatedUsing = OnRep_BeingRaided, BlueprintReadOnly, Category = "Ore Rush|Depot|Raid")
	bool bBeingRaided = false;

	UFUNCTION()
	void OnRep_BeingRaided();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Depot|Raid")
	void OnRaidStateChanged(bool bRaided);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
						UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
						bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastOnDepositFX(int32 Amount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Depot")
	void OnDepositFX(int32 Amount);

private:
	void RaidTick();
	void EndRaid(bool bBusted);
	bool IsOwnerPresent() const;
	int32 CountTeamDefensesNear() const;
	void SendAlarmToOwner(const FString& Message) const;
	ADepotZone* FindDepotForTeam(ETeam InTeam) const;

	TWeakObjectPtr<AOreRushCharacter> CurrentRaider;
	int32 StolenWorthThisRaid = 0;
	float RaidClosedUntil = -1000.f;
	FTimerHandle RaidTimerHandle;
};
