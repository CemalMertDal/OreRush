#include "Defense/ProjectileBullet.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

AProjectileBullet::AProjectileBullet()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	InitialLifeSpan = 1.5f;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.15f, 0.15f, 0.15f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->InitialSpeed = Speed;
	Movement->MaxSpeed = Speed;
	Movement->ProjectileGravityScale = 0.f;
	Movement->bAutoActivate = true;
}

void AProjectileBullet::InitVelocity(const FVector& Direction)
{
	if (Movement)
	{
		Movement->Velocity = Direction.GetSafeNormal() * Speed;
	}
}
