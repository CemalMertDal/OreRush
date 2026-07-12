
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "Interaction/OreRushInteractable.h"
#include "OreVein.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UStaticMesh;
class AOreRushCharacter;
class USoundBase;

UCLASS()
class ORERUSH_API AOreVein : public AActor, public IOreRushInteractable
{
	GENERATED_BODY()

public:
	AOreVein();

	virtual bool CanInteract(AOreRushCharacter* User) const override;
	virtual void ServerStartInteract(AOreRushCharacter* User) override;
	virtual void ServerStopInteract(AOreRushCharacter* User) override;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_OreType, BlueprintReadOnly, Category = "Ore Rush|Vein")
	EOreType OreType = EOreType::Iron;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Vein")
	TMap<EOreType, TObjectPtr<UStaticMesh>> OreMeshes;

	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Category = "Ore Rush|Vein")
	bool bUnlimited = true;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_RemainingUnits, BlueprintReadOnly, Category = "Ore Rush|Vein", meta = (ClampMin = "0", EditCondition = "!bUnlimited"))
	int32 RemainingUnits = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein", meta = (ClampMin = "0.05"))
	float MineTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Vein")
	TMap<EOreType, float> MineTimes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Vein|Sound")
	TObjectPtr<USoundBase> HitSound;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Vein")
	float GetEffectiveMineTime() const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Vein")
	bool CanBeMined() const { return !bDepleted && (bUnlimited || RemainingUnits > 0); }

	EOreType ServerExtractOne();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_OreType();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein")
	TObjectPtr<UBoxComponent> InteractBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Vein")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UFUNCTION()
	void OnRep_RemainingUnits();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMineHit();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Vein")
	void OnMineHitFX();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Vein")
	void OnDepletedFX();

private:
	void Deplete();

	void MineTick();

	void ApplyMeshForType();

	bool bDepleted = false;

	TWeakObjectPtr<AOreRushCharacter> CurrentMiner;
	FTimerHandle MineTimerHandle;
};
