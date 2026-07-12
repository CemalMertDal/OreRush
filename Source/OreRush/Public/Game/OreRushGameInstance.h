#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OreRushGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FOreRushSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Ore Rush|Steam")
	int32 Index = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ore Rush|Steam")
	FString OwnerName;

	UPROPERTY(BlueprintReadOnly, Category = "Ore Rush|Steam")
	int32 Ping = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ore Rush|Steam")
	int32 OpenSlots = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ore Rush|Steam")
	int32 MaxSlots = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Ore Rush|Steam")
	FString ServerName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOreRushOnCreateComplete,  bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOreRushOnFindComplete,    bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOreRushOnJoinComplete,    bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOreRushOnDestroyComplete, bool, bSuccess);

UCLASS()
class ORERUSH_API UOreRushGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Network")
	void HostGame(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Network")
	void JoinGame(const FString& IpAddress);

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Network")
	void ReturnToMenu(const FString& MenuMap);

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Network")
	void LeaveToMenu();

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Steam")
	void HostSteamSession(const FString& ServerName, bool bIsPrivate, int32 MaxPlayers = 2);

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Steam")
	void FindSteamSessions();

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Steam")
	void JoinFoundSession(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Ore Rush|Steam")
	void DestroySteamSession();

	UFUNCTION(BlueprintPure, Category = "Ore Rush|Steam")
	const TArray<FOreRushSessionInfo>& GetFoundSessions() const { return FoundSessions; }

	UPROPERTY(BlueprintAssignable, Category = "Ore Rush|Steam")
	FOreRushOnCreateComplete OnCreateComplete;

	UPROPERTY(BlueprintAssignable, Category = "Ore Rush|Steam")
	FOreRushOnFindComplete OnFindComplete;

	UPROPERTY(BlueprintAssignable, Category = "Ore Rush|Steam")
	FOreRushOnJoinComplete OnJoinComplete;

	UPROPERTY(BlueprintAssignable, Category = "Ore Rush|Steam")
	FOreRushOnDestroyComplete OnDestroyComplete;

	UPROPERTY(BlueprintReadWrite, Category = "Ore Rush|Cosmetic")
	uint8 SelectedColor = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Network")
	FString DefaultGameMap = TEXT("/Game/Maps/MainMap");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ore Rush|Network")
	FString DefaultMenuMap = TEXT("/Game/Maps/MainMenu");

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ore Rush|Network")
	FString LobbyMapPath = TEXT("/Game/Maps/Lobby");

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SearchSettings;
	TArray<FOreRushSessionInfo> FoundSessions;

	FDelegateHandle CreateHandle;
	FDelegateHandle FindHandle;
	FDelegateHandle JoinHandle;
	FDelegateHandle DestroyHandle;

	bool bWantsToHostAfterDestroy = false;
	bool bWantsMenuAfterDestroy = false;
	int32 PendingHostMaxPlayers = 2;
	FString PendingHostServerName;
	bool bPendingHostPrivate = false;

	void InternalCreateSession(const FString& ServerName, bool bIsPrivate, int32 MaxPlayers);

	void HandleCreateSessionComplete(FName SessionName, bool bSuccess);
	void HandleFindSessionsComplete(bool bSuccess);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bSuccess);
	void HandleInviteAccepted(const bool bWasSuccessful, const int32 ControllerId,
		FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult);

	bool IsUsingNullSubsystem() const;
};
