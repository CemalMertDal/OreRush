#include "PowerUp/PowerUpBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Character/OreRushCharacter.h"

APowerUpBase::APowerUpBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(80.f);
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Trigger);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Trigger);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 40.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}
}

void APowerUpBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &APowerUpBase::OnTriggerOverlap);
	}
}

void APowerUpBase::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!HasAuthority() || !bActive)
	{
		return;
	}

	AOreRushCharacter* User = Cast<AOreRushCharacter>(OtherActor);
	if (User == nullptr)
	{
		return;
	}

	ServerApplyTo(User);
	MulticastPickedUpFX();
	SetActiveState(false);

	if (RespawnTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &APowerUpBase::Respawn, RespawnTime, false);
	}
}

void APowerUpBase::ServerApplyTo(AOreRushCharacter* User)
{
}

void APowerUpBase::SetActiveState(bool bNewActive)
{
	bActive = bNewActive;
	SetActorEnableCollision(bNewActive);
	OnRep_Active();
}

void APowerUpBase::Respawn()
{
	SetActiveState(true);
}

void APowerUpBase::OnRep_Active()
{
	SetActorHiddenInGame(!bActive);
	OnActiveChanged(bActive);
}

void APowerUpBase::MulticastPickedUpFX_Implementation()
{
	OnPickedUpFX();
}

void APowerUpBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APowerUpBase, bActive);
}
