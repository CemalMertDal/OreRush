#include "Defense/DefenseBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Character/OreRushCharacter.h"
#include "Player/OreRushPlayerState.h"

ADefenseBase::ADefenseBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Range = CreateDefaultSubobject<USphereComponent>(TEXT("Range"));
	Range->InitSphereRadius(300.f);
	Range->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Range);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Range);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.4f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ADefenseBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Range->OnComponentBeginOverlap.AddDynamic(this, &ADefenseBase::OnRangeOverlap);
		GetWorldTimerManager().SetTimer(ArmTimerHandle, this, &ADefenseBase::Arm, FMath::Max(0.01f, ArmDelay), false);

		if (Lifetime > 0.f)
		{
			SetLifeSpan(Lifetime);
		}
	}
}

void ADefenseBase::Arm()
{
	bArmed = true;

	if (AOreRushCharacter* Enemy = FindEnemyInRange())
	{
		OnEnemyInRange(Enemy);
	}
}

void ADefenseBase::ServerInit(ETeam InOwnerTeam)
{
	if (HasAuthority())
	{
		OwnerTeam = InOwnerTeam;
	}
}

void ADefenseBase::OnRangeOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!HasAuthority() || !bArmed)
	{
		return;
	}

	AOreRushCharacter* Enemy = Cast<AOreRushCharacter>(OtherActor);
	if (Enemy == nullptr)
	{
		return;
	}

	ETeam EnemyTeam = ETeam::None;
	if (const AOreRushPlayerState* PS = Enemy->GetPlayerState<AOreRushPlayerState>())
	{
		EnemyTeam = PS->GetTeam();
	}

	if (EnemyTeam == ETeam::None || EnemyTeam == OwnerTeam)
	{
		return;
	}

	OnEnemyInRange(Enemy);
}

void ADefenseBase::OnEnemyInRange(AOreRushCharacter* Enemy)
{
	MulticastDefenseFX();
}

AOreRushCharacter* ADefenseBase::FindEnemyInRange() const
{
	if (Range == nullptr)
	{
		return nullptr;
	}

	TArray<AActor*> Overlapping;
	Range->GetOverlappingActors(Overlapping, AOreRushCharacter::StaticClass());
	for (AActor* A : Overlapping)
	{
		AOreRushCharacter* C = Cast<AOreRushCharacter>(A);
		if (C == nullptr)
		{
			continue;
		}
		const AOreRushPlayerState* PS = C->GetPlayerState<AOreRushPlayerState>();
		if (PS && PS->GetTeam() != ETeam::None && PS->GetTeam() != OwnerTeam)
		{
			return C;
		}
	}
	return nullptr;
}

void ADefenseBase::MulticastDefenseFX_Implementation()
{
	OnDefenseFX();
}

void ADefenseBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADefenseBase, OwnerTeam);
}
