#include "Player/OreRushPlayerController.h"
#include "Player/OreRushPlayerState.h"
#include "Game/OreRushGameMode.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

void AOreRushPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		if (BackgroundMusic)
		{
			MusicAudioComponent = UGameplayStatics::SpawnSound2D(this, BackgroundMusic, 1.0f, 1.0f, 0.0f, nullptr, true);
		}
	}
}

void AOreRushPlayerController::ClientShowAlarm_Implementation(const FString& Message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, Message);
	}
	OnAlarm(Message);
}

void AOreRushPlayerController::ServerSetColorIndex_Implementation(uint8 NewIndex)
{
	if (AOreRushPlayerState* PS = GetPlayerState<AOreRushPlayerState>())
	{
		PS->SetColorIndex(NewIndex);
	}
}

void AOreRushPlayerController::ServerRequestRestart_Implementation()
{
	if (AOreRushGameMode* GM = GetWorld()->GetAuthGameMode<AOreRushGameMode>())
	{
		GM->RestartMatch();
	}
}
