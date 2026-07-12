#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/OreRushTypes.h"
#include "DefenseBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class AOreRushCharacter;

UCLASS(Abstract)
class ORERUSH_API ADefenseBase : public AActor
{
	GENERATED_BODY()

public:
	ADefenseBase();

	void ServerInit(ETeam InOwnerTeam);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Ore Rush|Defense")
	ETeam OwnerTeam = ETeam::None;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnEnemyInRange(AOreRushCharacter* Enemy);

	AOreRushCharacter* FindEnemyInRange() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Defense")
	TObjectPtr<USphereComponent> Range;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Defense")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Defense", meta = (ClampMin = "0.0"))
	float ArmDelay = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Defense", meta = (ClampMin = "0.0"))
	float Lifetime = 45.f;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDefenseFX();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Defense")
	void OnDefenseFX();

private:
	UFUNCTION()
	void OnRangeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	void Arm();

	bool bArmed = false;
	FTimerHandle ArmTimerHandle;
};
