#pragma once

#include "CoreMinimal.h"
#include "Defense/DefenseBase.h"
#include "Turret.generated.h"

class AProjectileBullet;

UCLASS()
class ORERUSH_API ATurret : public ADefenseBase
{
	GENERATED_BODY()

public:
	ATurret();

protected:
	virtual void OnEnemyInRange(AOreRushCharacter* Enemy) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Defense", meta = (ClampMin = "0.0"))
	float StunDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Defense", meta = (ClampMin = "0.0"))
	float ReArmTime = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Defense")
	TSubclassOf<AProjectileBullet> ProjectileClass;

private:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFire(FVector Target);

	void ReArm();

	bool bReady = true;
	FTimerHandle CooldownHandle;
};
