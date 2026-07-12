#include "Defense/Turret.h"
#include "Defense/ProjectileBullet.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/OreRushCharacter.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATurret::ATurret()
{
	if (Range)
	{
		Range->SetSphereRadius(450.f);
	}
	if (Mesh)
	{
		Mesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.8f));
		Mesh->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	}
}

void ATurret::OnEnemyInRange(AOreRushCharacter* Enemy)
{
	if (!bReady || Enemy == nullptr)
	{
		return;
	}

	Enemy->ServerApplyStun(StunDuration);

	MulticastFire(Enemy->GetActorLocation());

	bReady = false;
	GetWorldTimerManager().SetTimer(CooldownHandle, this, &ATurret::ReArm, FMath::Max(0.1f, ReArmTime), false);
}

void ATurret::MulticastFire_Implementation(FVector Target)
{
	if (Mesh)
	{
		const FRotator Look = (Target - Mesh->GetComponentLocation()).Rotation();
		Mesh->SetWorldRotation(FRotator(0.f, Look.Yaw, 0.f));
	}

	OnDefenseFX();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	TSubclassOf<AProjectileBullet> Cls = ProjectileClass;
	if (!Cls)
	{
		Cls = AProjectileBullet::StaticClass();
	}

	const FVector Muzzle = GetActorLocation() + FVector(0.f, 0.f, 110.f);
	const FVector Dir = (Target - Muzzle).GetSafeNormal();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AProjectileBullet* Bullet = World->SpawnActor<AProjectileBullet>(Cls, Muzzle, Dir.Rotation(), SpawnParams))
	{
		Bullet->InitVelocity(Dir);
	}
}

void ATurret::ReArm()
{
	bReady = true;

	if (AOreRushCharacter* Enemy = FindEnemyInRange())
	{
		OnEnemyInRange(Enemy);
	}
}
