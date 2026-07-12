
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/OreRushTypes.h"
#include "Interaction/OreRushInteractable.h"
#include "OreRushCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuffChanged, EOreRushBuff, Buff, bool, bActive);

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UWalletComponent;
class UBuildComponent;
class AOreVein;
class UMaterialInterface;
struct FInputActionValue;

UCLASS()
class ORERUSH_API AOreRushCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AOreRushCharacter();

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Dash")
	float GetDashCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Components")
	UWalletComponent* GetWallet() const { return Wallet; }

	void ServerApplyStun(float Duration);

	void ServerApplySlow(float Mult, float Duration);

	void NotifyInteractFinished(UObject* Source);

	void ApplyTeamMaterials(ETeam Team);

	void ServerApplySpeedBuff(float Mult, float Duration);
	void ServerApplyMiningBuff(float Mult, float Duration);
	void ServerApplyShield(float Duration);

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Status")
	bool IsShielded() const { return bShielded; }

	float GetMiningSpeedMultiplier() const { return MiningSpeedMultiplier; }

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Status")
	AOreRushCharacter* GetEnemyCharacter() const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Status")
	float GetShieldRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Status")
	float GetSpeedBuffRemaining() const;

	UPROPERTY(BlueprintAssignable, Category = "Ore Rush|Status")
	FOnBuffChanged OnBuffChanged;

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Status")
	bool IsStunned() const { return bStunned; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Team")
	TArray<TObjectPtr<UMaterialInterface>> RedTeamMaterials;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Team")
	TArray<TObjectPtr<UMaterialInterface>> BlueTeamMaterials;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Components")
	TObjectPtr<UWalletComponent> Wallet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ore Rush|Components")
	TObjectPtr<UBuildComponent> Build;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> MineAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> PlaceTrapAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input")
	TObjectPtr<UInputAction> CycleTrapAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Input", meta = (ClampMin = "0.0"))
	float LookSensitivity = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Dash", meta = (ClampMin = "0.0"))
	float DashImpulse = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Dash", meta = (ClampMin = "0.0"))
	float DashVerticalImpulse = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Dash", meta = (ClampMin = "0.0"))
	float DashCooldown = 1.5f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Dash")
	void OnDashFX();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Mining", meta = (ClampMin = "0.0"))
	float MineRange = 400.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsMining, BlueprintReadOnly, Category = "Ore Rush|Mining")
	bool bIsMining = false;

	UFUNCTION()
	void OnRep_IsMining();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Mining")
	void OnMiningStateChanged(bool bMining);

	UPROPERTY(ReplicatedUsing = OnRep_Stunned, BlueprintReadOnly, Category = "Ore Rush|Status")
	bool bStunned = false;

	UFUNCTION()
	void OnRep_Stunned();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Status")
	void OnStunStateChanged(bool bInStunned);

	UPROPERTY(ReplicatedUsing = OnRep_Slow, BlueprintReadOnly, Category = "Ore Rush|Status")
	float SlowMultiplier = 1.f;

	UFUNCTION()
	void OnRep_Slow();

	UPROPERTY(ReplicatedUsing = OnRep_SpeedBuff, BlueprintReadOnly, Category = "Ore Rush|Status")
	float BuffSpeedMultiplier = 1.f;

	UFUNCTION()
	void OnRep_SpeedBuff();

	UPROPERTY(ReplicatedUsing = OnRep_Shield, BlueprintReadOnly, Category = "Ore Rush|Status")
	bool bShielded = false;

	UFUNCTION()
	void OnRep_Shield();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ore Rush|Status")
	void OnShieldStateChanged(bool bInShielded);

	UPROPERTY(ReplicatedUsing = OnRep_MiningBuff, BlueprintReadOnly, Category = "Ore Rush|Status")
	bool bMiningBuffActive = false;

	UFUNCTION()
	void OnRep_MiningBuff();

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void DashInput();

	void PlaceTrapInput();

	void CycleTrapInput();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDash(FVector DashDirection);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDashFX();

	void StartInteract();
	void StopInteract();

	UFUNCTION(Server, Reliable)
	void ServerStartInteract();

	UFUNCTION(Server, Reliable)
	void ServerStopInteract();

	void StopCurrentInteract();

	AActor* TraceForInteractable() const;

	UFUNCTION()
	void UpdateCarrySpeed();

	void RefreshTeamMaterial();

	UFUNCTION(Server, Reliable)
	void ServerPlaceTrap();

	UFUNCTION(Server, Reliable)
	void ServerCycleTrap();

	void ClearStun();

	void ClearSlow();

	void ClearSpeedBuff();
	void ClearMiningBuff();
	void ClearShield();

	bool IsMatchOver() const;

	float MiningSpeedMultiplier = 1.f;

	UPROPERTY()
	TScriptInterface<IOreRushInteractable> CurrentInteractable;

	FTimerHandle StunTimerHandle;
	FTimerHandle SlowTimerHandle;
	FTimerHandle SpeedBuffTimerHandle;
	FTimerHandle MiningBuffTimerHandle;
	FTimerHandle ShieldTimerHandle;
	FTimerHandle TeamMaterialRetryTimer;
	int32 TeamMaterialRetryCount = 0;
	float BaseWalkSpeed = 500.f;

	FVector LastMoveWorldDir = FVector::ZeroVector;

	float LastDashRequestTime = -1000.f;

	float LastDashTime = -1000.f;
};
