#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyStartZone.generated.h"

class UBoxComponent;
class AOreRushCharacter;

UCLASS()
class ORERUSH_API ALobbyStartZone : public AActor
{
	GENERATED_BODY()

public:
	ALobbyStartZone();

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Lobby")
	int32 GetCountdownSeconds() const { return CountdownSeconds; }

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Lobby")
	bool IsCounting() const { return bCounting; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Lobby")
	TObjectPtr<UBoxComponent> Zone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Lobby", meta = (ClampMin = "1"))
	int32 StartSeconds = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Lobby", meta = (ClampMin = "1"))
	int32 RequiredPlayers = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ore Rush|Lobby")
	FString GameMapPath = TEXT("/Game/Maps/MainMap");

	UPROPERTY(ReplicatedUsing = OnRep_Countdown, BlueprintReadOnly, Category = "Ore Rush|Lobby")
	int32 CountdownSeconds = 5;

	UPROPERTY(ReplicatedUsing = OnRep_Counting, BlueprintReadOnly, Category = "Ore Rush|Lobby")
	bool bCounting = false;

	UFUNCTION()
	void OnRep_Countdown();

	UFUNCTION()
	void OnRep_Counting();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Lobby")
	void OnCountdownChanged(int32 Seconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Lobby")
	void OnCountingChanged(bool bActive);

private:
	UFUNCTION()
	void OnZoneBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnZoneEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void Recompute();
	void StartCountdown();
	void CancelCountdown();
	void Tick1s();

	TArray<TWeakObjectPtr<AOreRushCharacter>> Inside;
	FTimerHandle CountdownTimer;
};
