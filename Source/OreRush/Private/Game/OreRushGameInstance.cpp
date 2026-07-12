#include "Game/OreRushGameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UOreRushGameInstance::Init() {
  Super::Init();

  if (IOnlineSubsystem *Subsystem = IOnlineSubsystem::Get()) {
    SessionInterface = Subsystem->GetSessionInterface();
    UE_LOG(LogTemp, Log, TEXT("[OreRush] OnlineSubsystem: %s"),
           *Subsystem->GetSubsystemName().ToString());

    if (SessionInterface.IsValid()) {
      SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
          FOnSessionUserInviteAcceptedDelegate::CreateUObject(
              this, &UOreRushGameInstance::HandleInviteAccepted));
    }
  } else {
    UE_LOG(
        LogTemp, Warning,
        TEXT("[OreRush] OnlineSubsystem yok — IP/LAN fallback kullanilacak."));
  }
}

void UOreRushGameInstance::HostGame(const FString &MapName) {
  const FString Map = MapName.IsEmpty() ? LobbyMapPath : MapName;
  UGameplayStatics::OpenLevel(this, FName(*Map), true, TEXT("listen"));
}

void UOreRushGameInstance::JoinGame(const FString &IpAddress) {
  if (IpAddress.IsEmpty()) {
    return;
  }

  if (APlayerController *PC = GetFirstLocalPlayerController()) {
    PC->ClientTravel(IpAddress, ETravelType::TRAVEL_Absolute);
  }
}

void UOreRushGameInstance::ReturnToMenu(const FString &MenuMap) {
  const FString Map = MenuMap.IsEmpty() ? DefaultMenuMap : MenuMap;
  UGameplayStatics::OpenLevel(this, FName(*Map), true);
}

void UOreRushGameInstance::LeaveToMenu() {
  if (SessionInterface.IsValid() &&
      SessionInterface->GetNamedSession(NAME_GameSession) != nullptr) {
    bWantsMenuAfterDestroy = true;
    DestroySteamSession();
    return;
  }
  UGameplayStatics::OpenLevel(this, FName(*DefaultMenuMap), true);
}

bool UOreRushGameInstance::IsUsingNullSubsystem() const {
  IOnlineSubsystem *Subsystem = IOnlineSubsystem::Get();
  return !Subsystem || Subsystem->GetSubsystemName() == TEXT("NULL");
}

void UOreRushGameInstance::HostSteamSession(const FString& ServerName, bool bIsPrivate, int32 MaxPlayers) {
  if (ServerName.TrimStartAndEnd().IsEmpty()) {
    OnCreateComplete.Broadcast(false);
    return;
  }

  if (!SessionInterface.IsValid()) {
    OnCreateComplete.Broadcast(false);
    return;
  }

  if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr) {
    bWantsToHostAfterDestroy = true;
    PendingHostMaxPlayers = MaxPlayers;
    PendingHostServerName = ServerName.TrimStartAndEnd();
    bPendingHostPrivate = bIsPrivate;
    DestroySteamSession();
    return;
  }

  InternalCreateSession(ServerName.TrimStartAndEnd(), bIsPrivate, MaxPlayers);
}

void UOreRushGameInstance::InternalCreateSession(const FString& ServerName, bool bIsPrivate, int32 MaxPlayers) {
  FOnlineSessionSettings Settings;
  Settings.bIsLANMatch = IsUsingNullSubsystem();
  Settings.NumPublicConnections = FMath::Max(1, MaxPlayers);
  Settings.bShouldAdvertise = !bIsPrivate;
  Settings.bAllowJoinInProgress = true;
  Settings.bUsesPresence = true;
  Settings.bAllowJoinViaPresence = true;
  Settings.bAllowInvites = true;
  Settings.bUseLobbiesIfAvailable = true;
  Settings.Set(FName(TEXT("SERVERNAME")), ServerName,
               EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
  Settings.Set(FName(TEXT("MAPNAME")), FString(TEXT("MainMap")),
               EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

  CreateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
      FOnCreateSessionCompleteDelegate::CreateUObject(
          this, &UOreRushGameInstance::HandleCreateSessionComplete));

  if (!SessionInterface->CreateSession(0, NAME_GameSession, Settings)) {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
    OnCreateComplete.Broadcast(false);
  }
}

void UOreRushGameInstance::HandleCreateSessionComplete(FName SessionName,
                                                       bool bSuccess) {
  if (SessionInterface.IsValid()) {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
  }

  OnCreateComplete.Broadcast(bSuccess);

  if (bSuccess) {
    if (UWorld *World = GetWorld()) {
      World->ServerTravel(LobbyMapPath + TEXT("?listen"));
    }
  }
}

void UOreRushGameInstance::FindSteamSessions() {
  if (!SessionInterface.IsValid()) {
    OnFindComplete.Broadcast(false);
    return;
  }

  SearchSettings = MakeShareable(new FOnlineSessionSearch());
  SearchSettings->bIsLanQuery = IsUsingNullSubsystem();
  SearchSettings->MaxSearchResults = 50;
  SearchSettings->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true,
                                    EOnlineComparisonOp::Equals);

  FindHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
      FOnFindSessionsCompleteDelegate::CreateUObject(
          this, &UOreRushGameInstance::HandleFindSessionsComplete));

  if (!SessionInterface->FindSessions(0, SearchSettings.ToSharedRef())) {
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
    OnFindComplete.Broadcast(false);
  }
}

void UOreRushGameInstance::HandleFindSessionsComplete(bool bSuccess) {
  if (SessionInterface.IsValid()) {
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
  }

  FoundSessions.Reset();

  if (bSuccess && SearchSettings.IsValid()) {
    for (int32 i = 0; i < SearchSettings->SearchResults.Num(); ++i) {
      const FOnlineSessionSearchResult &Result =
          SearchSettings->SearchResults[i];

      FOreRushSessionInfo Info;
      Info.Index = i;
      Info.OwnerName = Result.Session.OwningUserName;
      Info.Ping = Result.PingInMs;
      Info.MaxSlots = Result.Session.SessionSettings.NumPublicConnections;
      Info.OpenSlots = Result.Session.NumOpenPublicConnections;
      if (!Result.Session.SessionSettings.Get(FName(TEXT("SERVERNAME")), Info.ServerName) || Info.ServerName.IsEmpty()) {
        Info.ServerName = Result.Session.OwningUserName;
      }
      FoundSessions.Add(Info);
      UE_LOG(LogTemp, Log, TEXT("[OreRush] Session[%d] ServerName='%s' Owner='%s' Ping=%d Open=%d/%d"),
        i, *Info.ServerName, *Info.OwnerName, Info.Ping, Info.OpenSlots, Info.MaxSlots);
    }
  }

  OnFindComplete.Broadcast(bSuccess);
}

void UOreRushGameInstance::JoinFoundSession(int32 Index) {
  if (!SessionInterface.IsValid() || !SearchSettings.IsValid() ||
      !SearchSettings->SearchResults.IsValidIndex(Index)) {
    OnJoinComplete.Broadcast(false);
    return;
  }

  JoinHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
      FOnJoinSessionCompleteDelegate::CreateUObject(
          this, &UOreRushGameInstance::HandleJoinSessionComplete));

  if (!SessionInterface->JoinSession(0, NAME_GameSession,
                                     SearchSettings->SearchResults[Index])) {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
    OnJoinComplete.Broadcast(false);
  }
}

void UOreRushGameInstance::HandleJoinSessionComplete(
    FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
  if (SessionInterface.IsValid()) {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);
  }

  const bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);
  OnJoinComplete.Broadcast(bSuccess);

  if (!bSuccess) {
    return;
  }

  FString ConnectString;
  if (SessionInterface->GetResolvedConnectString(NAME_GameSession,
                                                 ConnectString)) {
    if (APlayerController *PC = GetFirstLocalPlayerController()) {
      PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
    }
  }
}

void UOreRushGameInstance::DestroySteamSession() {
  if (!SessionInterface.IsValid()) {
    OnDestroyComplete.Broadcast(false);
    return;
  }

  DestroyHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
      FOnDestroySessionCompleteDelegate::CreateUObject(
          this, &UOreRushGameInstance::HandleDestroySessionComplete));

  if (!SessionInterface->DestroySession(NAME_GameSession)) {
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
        DestroyHandle);
    OnDestroyComplete.Broadcast(false);
  }
}

void UOreRushGameInstance::HandleDestroySessionComplete(FName SessionName,
                                                        bool bSuccess) {
  if (SessionInterface.IsValid()) {
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
        DestroyHandle);
  }

  OnDestroyComplete.Broadcast(bSuccess);

  if (bWantsMenuAfterDestroy) {
    bWantsMenuAfterDestroy = false;
    UGameplayStatics::OpenLevel(this, FName(*DefaultMenuMap), true);
    return;
  }

  if (bSuccess && bWantsToHostAfterDestroy) {
    bWantsToHostAfterDestroy = false;
    InternalCreateSession(PendingHostServerName, bPendingHostPrivate, PendingHostMaxPlayers);
  }
}

void UOreRushGameInstance::HandleInviteAccepted(
    const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId,
    const FOnlineSessionSearchResult &InviteResult) {
  if (!bWasSuccessful || !SessionInterface.IsValid() ||
      !InviteResult.IsValid()) {
    return;
  }

  JoinHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
      FOnJoinSessionCompleteDelegate::CreateUObject(
          this, &UOreRushGameInstance::HandleJoinSessionComplete));

  SessionInterface->JoinSession(0, NAME_GameSession, InviteResult);
}
