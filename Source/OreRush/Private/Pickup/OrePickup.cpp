#include "Pickup/OrePickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Character/OreRushCharacter.h"
#include "Components/WalletComponent.h"
#include "Core/OreRushTypes.h"

AOrePickup::AOrePickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(70.f);
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Trigger);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Trigger);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}
}

void AOrePickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &AOrePickup::OnPickupOverlap);
		GetWorldTimerManager().SetTimer(ArmTimerHandle, this, &AOrePickup::Arm, FMath::Max(0.01f, ArmDelay), false);
	}
}

void AOrePickup::Arm()
{
	bArmed = true;
}

void AOrePickup::ServerInit(int32 Iron, int32 Gold, int32 Diamond)
{
	if (!HasAuthority())
	{
		return;
	}

	IronUnits = FMath::Max(0, Iron);
	GoldUnits = FMath::Max(0, Gold);
	DiamondUnits = FMath::Max(0, Diamond);
}

void AOrePickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!HasAuthority() || !bArmed)
	{
		return;
	}

	AOreRushCharacter* Char = Cast<AOreRushCharacter>(OtherActor);
	if (Char == nullptr)
	{
		return;
	}

	UWalletComponent* Wallet = Char->GetWallet();
	if (Wallet == nullptr || Wallet->IsFull())
	{
		return;
	}

	while (IronUnits > 0 && Wallet->ServerAddOre(EOreType::Iron))
	{
		IronUnits--;
	}
	while (GoldUnits > 0 && Wallet->ServerAddOre(EOreType::Gold))
	{
		GoldUnits--;
	}
	while (DiamondUnits > 0 && Wallet->ServerAddOre(EOreType::Diamond))
	{
		DiamondUnits--;
	}

	if (IronUnits + GoldUnits + DiamondUnits <= 0)
	{
		Destroy();
	}
}

void AOrePickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOrePickup, IronUnits);
	DOREPLIFETIME(AOrePickup, GoldUnits);
	DOREPLIFETIME(AOrePickup, DiamondUnits);
}
