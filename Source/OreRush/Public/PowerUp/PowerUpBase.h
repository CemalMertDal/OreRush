#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerUpBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AOreRushCharacter;

UCLASS(Abstract)
class ORERUSH_API APowerUpBase : public AActor
{
	GENERATED_BODY()

public:
	APowerUpBase();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void ServerApplyTo(AOreRushCharacter* User);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|PowerUp")
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|PowerUp")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|PowerUp", meta = (ClampMin = "0.0"))
	float RespawnTime = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|PowerUp", meta = (ClampMin = "0.0"))
	float BuffDuration = 8.f;

	UPROPERTY(ReplicatedUsing = OnRep_Active, BlueprintReadOnly, Category = "Ore Rush|PowerUp")
	bool bActive = true;

	UFUNCTION()
	void OnRep_Active();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|PowerUp")
	void OnActiveChanged(bool bIsActive);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|PowerUp")
	void OnPickedUpFX();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPickedUpFX();

private:
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	void SetActiveState(bool bNewActive);
	void Respawn();

	FTimerHandle RespawnTimerHandle;
};
