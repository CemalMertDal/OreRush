#include "Defense/Barrier.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

ABarrier::ABarrier()
{
	if (Range)
	{
		Range->SetSphereRadius(50.f);
	}
	if (Mesh)
	{
		Mesh->SetRelativeScale3D(FVector(3.0f, 0.5f, 2.0f));
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
		Mesh->SetCollisionProfileName(TEXT("BlockAll"));
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
