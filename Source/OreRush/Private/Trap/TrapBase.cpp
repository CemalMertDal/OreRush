#include "Trap/TrapBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Character/OreRushCharacter.h"
#include "Player/OreRushPlayerState.h"

ATrapBase::ATrapBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(60.f);
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Trigger);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Trigger);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(1.2f, 1.2f, 0.2f));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 10.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ATrapBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &ATrapBase::OnTriggerOverlap);
		GetWorldTimerManager().SetTimer(ArmTimerHandle, this, &ATrapBase::Arm, FMath::Max(0.01f, ArmDelay), false);

		if (Lifetime > 0.f)
		{
			SetLifeSpan(Lifetime);
		}
	}
}

void ATrapBase::Arm()
{
	bArmed = true;
}

void ATrapBase::ServerInit(ETeam InOwnerTeam)
{
	if (HasAuthority())
	{
		OwnerTeam = InOwnerTeam;
	}
}

void ATrapBase::OnTriggerOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!HasAuthority() || !bArmed || bTriggered)
	{
		return;
	}

	AOreRushCharacter* Victim = Cast<AOreRushCharacter>(OtherActor);
	if (Victim == nullptr)
	{
		return;
	}

	ETeam VictimTeam = ETeam::None;
	if (const AOreRushPlayerState* PS = Victim->GetPlayerState<AOreRushPlayerState>())
	{
		VictimTeam = PS->GetTeam();
	}

	if (VictimTeam == ETeam::None || VictimTeam == OwnerTeam)
	{
		return;
	}

	if (Victim->IsShielded())
	{
		return;
	}

	bTriggered = true;
	OnTriggered(Victim);
	MulticastTriggerFX();
	SetLifeSpan(0.2f);
}

void ATrapBase::OnTriggered(AOreRushCharacter* Victim)
{
}

void ATrapBase::MulticastTriggerFX_Implementation()
{
	OnTriggerFX();
}

void ATrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATrapBase, OwnerTeam);
}
