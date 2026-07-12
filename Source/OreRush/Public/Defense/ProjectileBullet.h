#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBullet.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class ORERUSH_API AProjectileBullet : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBullet();

	void InitVelocity(const FVector& Direction);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Projectile")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Projectile")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Projectile", meta = (ClampMin = "1.0"))
	float Speed = 4000.f;
};
