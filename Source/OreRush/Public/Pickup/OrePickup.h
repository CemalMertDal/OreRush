#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrePickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ORERUSH_API AOrePickup : public AActor
{
	GENERATED_BODY()

public:
	AOrePickup();

	void ServerInit(int32 Iron, int32 Gold, int32 Diamond);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Pickup")
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Pickup")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Pickup", meta = (ClampMin = "0.0"))
	float ArmDelay = 0.75f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Pickup")
	int32 IronUnits = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Pickup")
	int32 GoldUnits = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Pickup")
	int32 DiamondUnits = 0;

private:
	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	void Arm();

	bool bArmed = false;
	FTimerHandle ArmTimerHandle;
};
