#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SmokeCloud.generated.h"

class USceneComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class ORERUSH_API ASmokeCloud : public AActor
{
	GENERATED_BODY()

public:
	ASmokeCloud();

	void ServerInit(float Duration);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Smoke")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Smoke")
	TObjectPtr<UNiagaraComponent> SmokeFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Smoke")
	TObjectPtr<UNiagaraSystem> SmokeSystem;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Smoke", meta = (ClampMin = "0.1"))
	float SmokeScale = 8.f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Smoke")
	void OnSmokeFX();
};
