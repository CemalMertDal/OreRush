#include "Trap/DecoyOre.h"
#include "Character/OreRushCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

ADecoyOre::ADecoyOre()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded() && Mesh)
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
		Mesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, 30.f));
	}
}

void ADecoyOre::OnTriggered(AOreRushCharacter* Victim)
{
	if (Victim)
	{
		Victim->ServerApplyStun(StunDuration);
	}
}
