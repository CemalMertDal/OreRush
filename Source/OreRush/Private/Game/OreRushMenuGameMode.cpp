#include "Game/OreRushMenuGameMode.h"
#include "Player/OreRushMenuController.h"
#include "GameFramework/SpectatorPawn.h"

AOreRushMenuGameMode::AOreRushMenuGameMode()
{
	PlayerControllerClass = AOreRushMenuController::StaticClass();
	DefaultPawnClass = ASpectatorPawn::StaticClass();
}
