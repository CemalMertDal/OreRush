#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "OreRushInteractable.generated.h"

class AOreRushCharacter;

UINTERFACE(MinimalAPI)
class UOreRushInteractable : public UInterface
{
	GENERATED_BODY()
};

class IOreRushInteractable
{
	GENERATED_BODY()

public:
	virtual bool CanInteract(AOreRushCharacter* User) const = 0;
	virtual void ServerStartInteract(AOreRushCharacter* User) = 0;
	virtual void ServerStopInteract(AOreRushCharacter* User) = 0;
};
