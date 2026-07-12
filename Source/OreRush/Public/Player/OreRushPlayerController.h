
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OreRushPlayerController.generated.h"

class USoundBase;
class UAudioComponent;

UCLASS()
class ORERUSH_API AOreRushPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(Client, Reliable)
	void ClientShowAlarm(const FString& Message);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Ore Rush|Cosmetic")
	void ServerSetColorIndex(uint8 NewIndex);

	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "Ore Rush|Match")
	void ServerRequestRestart();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Sound")
	TObjectPtr<USoundBase> BackgroundMusic;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Ore Rush|Sound")
	TObjectPtr<UAudioComponent> MusicAudioComponent;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Alarm")
	void OnAlarm(const FString& Message);
};
