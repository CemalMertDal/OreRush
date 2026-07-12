#include "Player/OreRushMenuController.h"
#include "Blueprint/UserWidget.h"

AOreRushMenuController::AOreRushMenuController()
{
	bShowMouseCursor = true;
}

void AOreRushMenuController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	if (MenuWidgetClass)
	{
		MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetClass);
		if (MenuWidget)
		{
			MenuWidget->AddToViewport();
		}
	}

	bShowMouseCursor = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (MenuWidget)
	{
		InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
	}
	SetInputMode(InputMode);
}
