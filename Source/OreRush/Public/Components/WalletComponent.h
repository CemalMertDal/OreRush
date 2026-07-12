
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/OreRushTypes.h"
#include "WalletComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWalletChanged);

UCLASS(ClassGroup = (OreRush), meta = (BlueprintSpawnableComponent))
class ORERUSH_API UWalletComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWalletComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Wallet", meta = (ClampMin = "1"))
	int32 Capacity = 15;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Wallet", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float FullSpeedMultiplier = 0.55f;

	UPROPERTY(BlueprintAssignable, Category = "Ore Rush|Wallet")
	FOnWalletChanged OnWalletChanged;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	int32 GetCount(EOreType Type) const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	int32 GetTotalUnits() const { return IronCount + GoldCount + DiamondCount; }

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	int32 GetTotalWorth() const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	int32 GetEffectiveCapacity() const { return Capacity + BonusCapacity; }

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	bool IsFull() const { return GetTotalWorth() >= GetEffectiveCapacity(); }

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Wallet")
	float GetSpeedMultiplier() const;

	bool ServerAddOre(EOreType Type);

	int32 ServerClear();

	bool ServerSpendWorth(int32 Cost);

	void ServerTakeAll(int32& OutIron, int32& OutGold, int32& OutDiamond);

	void ServerApplyCapacityBonus(int32 Amount, float Duration);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Wallet, BlueprintReadOnly, Category = "Ore Rush|Wallet")
	int32 IronCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Wallet, BlueprintReadOnly, Category = "Ore Rush|Wallet")
	int32 GoldCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Wallet, BlueprintReadOnly, Category = "Ore Rush|Wallet")
	int32 DiamondCount = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Wallet, BlueprintReadOnly, Category = "Ore Rush|Wallet")
	int32 BonusCapacity = 0;

	UFUNCTION()
	void OnRep_Wallet();

private:
	void ClearCapacityBonus();

	int32* CountPtr(EOreType Type);

	FTimerHandle CapacityTimerHandle;
};
