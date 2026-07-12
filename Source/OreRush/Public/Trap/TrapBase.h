#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "TrapBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AOreRushCharacter;

UCLASS(Abstract)
class ORERUSH_API ATrapBase : public AActor
{
	GENERATED_BODY()

public:
	ATrapBase();

	void ServerInit(ETeam InOwnerTeam);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Trap")
	ETeam OwnerTeam = ETeam::None;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnTriggered(AOreRushCharacter* Victim);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Trap")
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Trap")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap", meta = (ClampMin = "0.0"))
	float ArmDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Trap", meta = (ClampMin = "0.0"))
	float Lifetime = 30.f;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastTriggerFX();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Trap")
	void OnTriggerFX();

private:
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	void Arm();

	bool bArmed = false;
	bool bTriggered = false;
	FTimerHandle ArmTimerHandle;
};
