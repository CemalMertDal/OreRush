#include "Trap/SmokeCloud.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

ASmokeCloud::ASmokeCloud()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SmokeFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeFX"));
	SmokeFX->SetupAttachment(Root);
	SmokeFX->SetAutoActivate(false);
}

void ASmokeCloud::BeginPlay()
{
	Super::BeginPlay();

	if (SmokeSystem)
	{
		SmokeFX->SetAsset(SmokeSystem);
		SmokeFX->SetRelativeScale3D(FVector(SmokeScale));
		SmokeFX->Activate(true);
	}

	OnSmokeFX();
}

void ASmokeCloud::ServerInit(float Duration)
{
	if (HasAuthority() && Duration > 0.f)
	{
		SetLifeSpan(Duration);
	}
}
