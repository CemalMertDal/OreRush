
#include "Character/OreRushCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/WalletComponent.h"
#include "Components/BuildComponent.h"
#include "Ore/OreVein.h"
#include "Player/OreRushPlayerState.h"
#include "Game/OreRushGameState.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"

AOreRushCharacter::AOreRushCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 500.f, 0.f);
	Move->JumpZVelocity = 500.f;
	Move->AirControl = 0.35f;
	Move->MaxWalkSpeed = 500.f;
	Move->MinAnalogWalkSpeed = 20.f;
	Move->BrakingDecelerationWalking = 2000.f;
	Move->BrakingDecelerationFalling = 1500.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Wallet = CreateDefaultSubobject<UWalletComponent>(TEXT("Wallet"));

	Build = CreateDefaultSubobject<UBuildComponent>(TEXT("Build"));
}

void AOreRushCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
			if (MouseLookMappingContext)
			{
				Subsystem->AddMappingContext(MouseLookMappingContext, 0);
			}
		}
	}

	BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	if (Wallet)
	{
		Wallet->OnWalletChanged.AddDynamic(this, &AOreRushCharacter::UpdateCarrySpeed);
	}

	RefreshTeamMaterial();
}

void AOreRushCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	RefreshTeamMaterial();
}

void AOreRushCharacter::RefreshTeamMaterial()
{
	const AOreRushPlayerState* PS = GetPlayerState<AOreRushPlayerState>();
	if (PS == nullptr || PS->GetTeam() == ETeam::None)
	{
		if (TeamMaterialRetryCount < 20)
		{
			TeamMaterialRetryCount++;
			GetWorldTimerManager().SetTimer(TeamMaterialRetryTimer, this, &AOreRushCharacter::RefreshTeamMaterial, 0.25f, false);
		}
		return;
	}

	ApplyTeamMaterials(PS->GetTeam());
}

void AOreRushCharacter::ApplyTeamMaterials(ETeam Team)
{
	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh == nullptr || Team == ETeam::None)
	{
		return;
	}

	const TArray<TObjectPtr<UMaterialInterface>>& Materials = (Team == ETeam::Red) ? RedTeamMaterials : BlueTeamMaterials;
	for (int32 i = 0; i < Materials.Num(); ++i)
	{
		if (Materials[i])
		{
			SkelMesh->SetMaterial(i, Materials[i]);
		}
	}
}

void AOreRushCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOreRushCharacter::Move);
		}
		if (LookAction)
		{
			EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOreRushCharacter::Look);
		}
		if (MouseLookAction)
		{
			EIC->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AOreRushCharacter::Look);
		}
		if (JumpAction)
		{
			EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}
		if (DashAction)
		{
			EIC->BindAction(DashAction, ETriggerEvent::Started, this, &AOreRushCharacter::DashInput);
		}
		if (MineAction)
		{
			EIC->BindAction(MineAction, ETriggerEvent::Started, this, &AOreRushCharacter::StartInteract);
			EIC->BindAction(MineAction, ETriggerEvent::Completed, this, &AOreRushCharacter::StopInteract);
		}
		if (PlaceTrapAction)
		{
			EIC->BindAction(PlaceTrapAction, ETriggerEvent::Started, this, &AOreRushCharacter::PlaceTrapInput);
		}
		if (CycleTrapAction)
		{
			EIC->BindAction(CycleTrapAction, ETriggerEvent::Started, this, &AOreRushCharacter::CycleTrapInput);
		}
	}
}

bool AOreRushCharacter::IsMatchOver() const
{
	const UWorld* W = GetWorld();
	const AOreRushGameState* GS = W ? W->GetGameState<AOreRushGameState>() : nullptr;
	return GS && GS->IsMatchEnded();
}

void AOreRushCharacter::Move(const FInputActionValue& Value)
{
	if (bIsMining || bStunned || IsMatchOver())
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();

	if (Controller == nullptr)
	{
		return;
	}

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);

	LastMoveWorldDir = (Forward * Axis.Y + Right * Axis.X).GetSafeNormal();
}

void AOreRushCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (Controller == nullptr)
	{
		return;
	}

	AddControllerYawInput(Axis.X * LookSensitivity);
	AddControllerPitchInput(Axis.Y * LookSensitivity);
}

void AOreRushCharacter::DashInput()
{
	if (bStunned || IsMatchOver())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastDashRequestTime < DashCooldown)
	{
		return;
	}
	LastDashRequestTime = Now;

	FVector Dir = GetVelocity().GetSafeNormal2D();
	if (Dir.IsNearlyZero())
	{
		Dir = LastMoveWorldDir.GetSafeNormal2D();
	}
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector().GetSafeNormal2D();
	}

	ServerDash(Dir);
}

bool AOreRushCharacter::ServerDash_Validate(FVector DashDirection)
{
	return !DashDirection.ContainsNaN();
}

void AOreRushCharacter::ServerDash_Implementation(FVector DashDirection)
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastDashTime < DashCooldown)
	{
		return;
	}
	LastDashTime = Now;

	FVector Dir = DashDirection.GetSafeNormal2D();
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector().GetSafeNormal2D();
	}

	const FVector LaunchVelocity = Dir * DashImpulse + FVector(0.f, 0.f, DashVerticalImpulse);

	LaunchCharacter(LaunchVelocity, /*bXYOverride=*/true, /*bZOverride=*/DashVerticalImpulse > 0.f);

	MulticastDashFX();
}

void AOreRushCharacter::MulticastDashFX_Implementation()
{
	OnDashFX();
}

float AOreRushCharacter::GetDashCooldownRemaining() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return 0.f;
	}

	const float Elapsed = World->GetTimeSeconds() - LastDashRequestTime;
	return FMath::Max(0.f, DashCooldown - Elapsed);
}


void AOreRushCharacter::StartInteract()
{
	ServerStartInteract();
}

void AOreRushCharacter::StopInteract()
{
	ServerStopInteract();
}

void AOreRushCharacter::ServerStartInteract_Implementation()
{
	if (bIsMining || bStunned || IsMatchOver())
	{
		return;
	}

	AActor* HitActor = TraceForInteractable();
	if (HitActor == nullptr)
	{
		return;
	}

	IOreRushInteractable* Interactable = Cast<IOreRushInteractable>(HitActor);
	if (Interactable == nullptr || !Interactable->CanInteract(this))
	{
		return;
	}

	CurrentInteractable.SetObject(HitActor);
	CurrentInteractable.SetInterface(Interactable);

	bIsMining = true;
	OnRep_IsMining();

	Interactable->ServerStartInteract(this);
}

void AOreRushCharacter::ServerStopInteract_Implementation()
{
	StopCurrentInteract();
}

void AOreRushCharacter::StopCurrentInteract()
{
	if (CurrentInteractable)
	{
		CurrentInteractable->ServerStopInteract(this);
	}
	CurrentInteractable = nullptr;

	if (bIsMining)
	{
		bIsMining = false;
		OnRep_IsMining();
	}
}

void AOreRushCharacter::NotifyInteractFinished(UObject* Source)
{
	if (CurrentInteractable.GetObject() != Source)
	{
		return;
	}

	CurrentInteractable = nullptr;
	if (bIsMining)
	{
		bIsMining = false;
		OnRep_IsMining();
	}
}

AActor* AOreRushCharacter::TraceForInteractable() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, BaseEyeHeight);
	const FVector End = Start + GetControlRotation().Vector() * MineRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult Hit;
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return Hit.GetActor();
	}
	return nullptr;
}

void AOreRushCharacter::OnRep_IsMining()
{
	if (bIsMining)
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

	OnMiningStateChanged(bIsMining);
}

void AOreRushCharacter::UpdateCarrySpeed()
{
	const float CarryMult = Wallet ? Wallet->GetSpeedMultiplier() : 1.f;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * CarryMult * SlowMultiplier * BuffSpeedMultiplier;
}

void AOreRushCharacter::PlaceTrapInput()
{
	ServerPlaceTrap();
}

void AOreRushCharacter::ServerPlaceTrap_Implementation()
{
	if (Build)
	{
		Build->ServerTryPlace(Build->GetSelectedIndex());
	}
}

void AOreRushCharacter::CycleTrapInput()
{
	ServerCycleTrap();
}

void AOreRushCharacter::ServerCycleTrap_Implementation()
{
	if (Build)
	{
		Build->ServerCycleSelection();
	}
}

void AOreRushCharacter::ServerApplyStun(float Duration)
{
	if (!HasAuthority() || Duration <= 0.f)
	{
		return;
	}

	bStunned = true;
	OnRep_Stunned();
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &AOreRushCharacter::ClearStun, Duration, false);
}

void AOreRushCharacter::ClearStun()
{
	if (!HasAuthority())
	{
		return;
	}

	bStunned = false;
	OnRep_Stunned();
}

void AOreRushCharacter::OnRep_Stunned()
{
	if (bStunned)
	{
		GetCharacterMovement()->StopMovementImmediately();
	}

	OnStunStateChanged(bStunned);
}

void AOreRushCharacter::ServerApplySlow(float Mult, float Duration)
{
	if (!HasAuthority() || Duration <= 0.f)
	{
		return;
	}

	SlowMultiplier = FMath::Clamp(Mult, 0.1f, 1.f);
	OnRep_Slow();
	GetWorldTimerManager().SetTimer(SlowTimerHandle, this, &AOreRushCharacter::ClearSlow, Duration, false);
}

void AOreRushCharacter::ClearSlow()
{
	if (!HasAuthority())
	{
		return;
	}

	SlowMultiplier = 1.f;
	OnRep_Slow();
}

void AOreRushCharacter::OnRep_Slow()
{
	UpdateCarrySpeed();
}

void AOreRushCharacter::ServerApplySpeedBuff(float Mult, float Duration)
{
	if (!HasAuthority() || Duration <= 0.f)
	{
		return;
	}

	BuffSpeedMultiplier = FMath::Max(0.1f, Mult);
	OnRep_SpeedBuff();
	GetWorldTimerManager().SetTimer(SpeedBuffTimerHandle, this, &AOreRushCharacter::ClearSpeedBuff, Duration, false);
}

void AOreRushCharacter::ClearSpeedBuff()
{
	if (!HasAuthority())
	{
		return;
	}

	BuffSpeedMultiplier = 1.f;
	OnRep_SpeedBuff();
}

void AOreRushCharacter::OnRep_SpeedBuff()
{
	UpdateCarrySpeed();
	OnBuffChanged.Broadcast(EOreRushBuff::Speed, !FMath::IsNearlyEqual(BuffSpeedMultiplier, 1.f));
}

void AOreRushCharacter::ServerApplyMiningBuff(float Mult, float Duration)
{
	if (!HasAuthority() || Duration <= 0.f)
	{
		return;
	}

	MiningSpeedMultiplier = FMath::Clamp(Mult, 0.1f, 1.f);
	bMiningBuffActive = true;
	OnRep_MiningBuff();
	GetWorldTimerManager().SetTimer(MiningBuffTimerHandle, this, &AOreRushCharacter::ClearMiningBuff, Duration, false);
}

void AOreRushCharacter::ClearMiningBuff()
{
	if (!HasAuthority())
	{
		return;
	}

	MiningSpeedMultiplier = 1.f;
	bMiningBuffActive = false;
	OnRep_MiningBuff();
}

void AOreRushCharacter::OnRep_MiningBuff()
{
	OnBuffChanged.Broadcast(EOreRushBuff::Mining, bMiningBuffActive);
}

void AOreRushCharacter::ServerApplyShield(float Duration)
{
	if (!HasAuthority() || Duration <= 0.f)
	{
		return;
	}

	bShielded = true;
	OnRep_Shield();
	GetWorldTimerManager().SetTimer(ShieldTimerHandle, this, &AOreRushCharacter::ClearShield, Duration, false);
}

void AOreRushCharacter::ClearShield()
{
	if (!HasAuthority())
	{
		return;
	}

	bShielded = false;
	OnRep_Shield();
}

void AOreRushCharacter::OnRep_Shield()
{
	OnShieldStateChanged(bShielded);
	OnBuffChanged.Broadcast(EOreRushBuff::Shield, bShielded);
}

AOreRushCharacter* AOreRushCharacter::GetEnemyCharacter() const
{
	ETeam MyTeam = ETeam::None;
	if (const AOreRushPlayerState* PS = GetPlayerState<AOreRushPlayerState>())
	{
		MyTeam = PS->GetTeam();
	}

	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	for (TActorIterator<AOreRushCharacter> It(World); It; ++It)
	{
		AOreRushCharacter* Other = *It;
		if (Other == this)
		{
			continue;
		}
		ETeam OtherTeam = ETeam::None;
		if (const AOreRushPlayerState* OPS = Other->GetPlayerState<AOreRushPlayerState>())
		{
			OtherTeam = OPS->GetTeam();
		}
		if (OtherTeam != ETeam::None && OtherTeam != MyTeam)
		{
			return Other;
		}
	}
	return nullptr;
}

float AOreRushCharacter::GetShieldRemaining() const
{
	return FMath::Max(0.f, GetWorldTimerManager().GetTimerRemaining(ShieldTimerHandle));
}

float AOreRushCharacter::GetSpeedBuffRemaining() const
{
	return FMath::Max(0.f, GetWorldTimerManager().GetTimerRemaining(SpeedBuffTimerHandle));
}

void AOreRushCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOreRushCharacter, bIsMining);
	DOREPLIFETIME(AOreRushCharacter, bStunned);
	DOREPLIFETIME(AOreRushCharacter, SlowMultiplier);
	DOREPLIFETIME(AOreRushCharacter, BuffSpeedMultiplier);
	DOREPLIFETIME(AOreRushCharacter, bShielded);
	DOREPLIFETIME(AOreRushCharacter, bMiningBuffActive);
}
