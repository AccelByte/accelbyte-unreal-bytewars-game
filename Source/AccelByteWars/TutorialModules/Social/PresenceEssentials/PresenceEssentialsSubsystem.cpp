// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PresenceEssentialsSubsystem.h"

#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"

#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UPresenceEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // On lobby, set level status to "In Match" and activity status to "In Lobby".
    UMatchLobbyWidget::OnEnterLobbyDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
    {
        LevelStatus = TEXT_PRESENCE_LEVEL_GAMEPLAY.ToString();
        ActivityStatus = TEXT_PRESENCE_ACTIVITY_LOBBY.ToString();

        UpdatePrimaryPlayerPresenceStatus();
    });

    // On world loaded, update level status.
    FWorldDelegates::OnPostWorldInitialization.AddWeakLambda(this, [this](UWorld* World, const UWorld::InitializationValues IVS)
    {
        if (!World) 
        {
            return;
        }

        World->OnWorldBeginPlay.RemoveAll(this);
        World->OnWorldBeginPlay.AddUObject(this, &ThisClass::OnLevelLoaded);
    });

    if (UAccelByteWarsOnlineSessionBase* OnlineSession = GetOnlineSession())
    {
        // On matchmaking started, set matchmaking status.
        if (FOnMatchmakingResponse* OnStartMatchmakingComplete = OnlineSession->GetOnStartMatchmakingCompleteDelegates())
        {
            OnStartMatchmakingComplete->AddWeakLambda(this, [this](FName SessionName, bool bSucceeded)
            {
                ActivityStatus = bSucceeded ? TEXT_PRESENCE_ACTIVITY_MATCHMAKING.ToString() : FString();
                UpdatePrimaryPlayerPresenceStatus();
            });
        }

        // On matchmaking complete, remove matchmaking status.
        if (FOnMatchmakingResponse* OnMatchmakingComplete = OnlineSession->GetOnMatchmakingCompleteDelegates())
        {
            OnMatchmakingComplete->AddWeakLambda(this, [this](FName SessionName, bool bSucceeded)
            {
                ActivityStatus = FString();
                UpdatePrimaryPlayerPresenceStatus();
            });
        }

        // On matchmaking canceled, remove matchmaking status.
        if (FOnMatchmakingResponse* OnCancelMatchmakingComplete = OnlineSession->GetOnCancelMatchmakingCompleteDelegates())
        {
            OnCancelMatchmakingComplete->AddWeakLambda(this, [this](FName SessionName, bool bSucceeded)
            {
                ActivityStatus = FString();
                UpdatePrimaryPlayerPresenceStatus();
            });
        }

        // On party session updated, update party presence status.
        if (FOnJoinSessionComplete* OnJoinPartyComplete = OnlineSession->GetOnJoinPartyCompleteDelegates())
        {
            OnJoinPartyComplete->AddWeakLambda(this, [this](FName SessionName, EOnJoinSessionCompleteResult::Type Result)
            {
                UpdatePrimaryPlayerPresenceStatus();
            });
        }
        if (FOnDestroySessionComplete* OnLeavePartyComplete = OnlineSession->GetOnLeavePartyCompleteDelegates())
        {
            OnLeavePartyComplete->AddWeakLambda(this, [this](FName SessionName, bool bSucceeded)
            {
                UpdatePrimaryPlayerPresenceStatus();
            });
        }
        if (FOnSessionParticipantsChange* OnPartyMemberChange = OnlineSession->GetOnPartyMembersChangeDelegates())
        {
            OnPartyMemberChange->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Member, bool bJoined)
            {
                UpdatePrimaryPlayerPresenceStatus();
            });
        }
        if (FOnSessionUpdateReceived* OnPartySessionUpdateReceived = OnlineSession->GetOnPartySessionUpdateReceivedDelegates())
        {
            OnPartySessionUpdateReceived->AddWeakLambda(this, [this](FName SessionName)
            {
                UpdatePrimaryPlayerPresenceStatus();
            });
        }
    }

    // Update presence status once the player logged in and connected to lobby.
    if (FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface()) 
    {
        IdentityInterface->AddOnConnectLobbyCompleteDelegate_Handle(0,
            FOnConnectLobbyCompleteDelegate::CreateWeakLambda(this, [this](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
            {
                UpdatePrimaryPlayerPresenceStatus();
            }
        ));
    }

    // Update presence when friend and blocked player list changed.
    if (FOnlineFriendsAccelBytePtr FriendsInterface = GetFriendsInterface())
    {
        FriendsInterface->AddOnFriendsChangeDelegate_Handle(0, FOnFriendsChangeDelegate::CreateUObject(this, &ThisClass::OnFriendListChange));
        FriendsInterface->AddOnBlockListChangeDelegate_Handle(0, FOnBlockListChangeDelegate::CreateUObject(this, &ThisClass::OnBlockedPlayerListChange));
    }

    // Listen to presence events.
    if (FOnlinePresenceAccelBytePtr PresenceInterface = GetPresenceInterface())
    {
        PresenceInterface->AddOnPresenceReceivedDelegate_Handle(FOnPresenceReceivedDelegate::CreateUObject(this, &ThisClass::OnPresenceReceived));
        PresenceInterface->AddOnBulkQueryPresenceCompleteDelegate_Handle(FOnBulkQueryPresenceCompleteDelegate::CreateUObject(this, &ThisClass::OnBulkQueryPresenceComplete));
    }
}

void UPresenceEssentialsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    // Clean-up cache.
    LevelStatus = FString();
    ActivityStatus = FString();

    // Unbind events.
    AAccelByteWarsGameState::OnInitialized.RemoveAll(this);
    UMatchLobbyWidget::OnEnterLobbyDelegate.RemoveAll(this);

    FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
    if (GetWorld())
    {
        GetWorld()->OnWorldBeginPlay.RemoveAll(this);
    }

    if (UAccelByteWarsOnlineSessionBase* OnlineSession = GetOnlineSession())
    {
        if (FOnMatchmakingResponse* OnStartMatchmakingComplete = OnlineSession->GetOnStartMatchmakingCompleteDelegates())
        {
            OnStartMatchmakingComplete->RemoveAll(this);
        }

        if (FOnMatchmakingResponse* OnMatchmakingComplete = OnlineSession->GetOnMatchmakingCompleteDelegates())
        {
            OnMatchmakingComplete->RemoveAll(this);
        }

        if (FOnMatchmakingResponse* OnCancelMatchmakingComplete = OnlineSession->GetOnCancelMatchmakingCompleteDelegates())
        {
            OnCancelMatchmakingComplete->RemoveAll(this);
        }

        if (FOnJoinSessionComplete* OnJoinPartyComplete = OnlineSession->GetOnJoinPartyCompleteDelegates())
        {
            OnJoinPartyComplete->RemoveAll(this);
        }

        if (FOnDestroySessionComplete* OnLeavePartyComplete = OnlineSession->GetOnLeavePartyCompleteDelegates())
        {
            OnLeavePartyComplete->RemoveAll(this);
        }

        if (FOnSessionParticipantsChange* OnPartyMemberChange = OnlineSession->GetOnPartyMembersChangeDelegates())
        {
            OnPartyMemberChange->RemoveAll(this);
        }

        if (FOnSessionUpdateReceived* OnPartySessionUpdateReceived = OnlineSession->GetOnPartySessionUpdateReceivedDelegates())
        {
            OnPartySessionUpdateReceived->RemoveAll(this);
        }
    }

    if (FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface())
    {
        IdentityInterface->ClearOnConnectLobbyCompleteDelegates(0, this);
    }

    if (FOnlineFriendsAccelBytePtr FriendsInterface = GetFriendsInterface())
    {
        FriendsInterface->ClearOnFriendsChangeDelegates(0, this);
        FriendsInterface->ClearOnBlockListChangeDelegates(0, this);
    }

    if (FOnlinePresenceAccelBytePtr PresenceInterface = GetPresenceInterface())
    {
        PresenceInterface->ClearOnPresenceReceivedDelegates(this);
        PresenceInterface->ClearOnBulkQueryPresenceCompleteDelegates(this);
    }
}

void UPresenceEssentialsSubsystem::UpdatePrimaryPlayerPresenceStatus()
{
    FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface();
    if (!IdentityInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to update current logged-in player's presence. Identity interface is invalid."));
        return;
    }

    const FUniqueNetIdPtr UserId = GetPrimaryPlayerUserId();
    if (!UserId)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to update current logged-in player's presence. User Id is invalid."));
        return;
    }

    // Abort if not logged-in.
    if (IdentityInterface->GetLoginStatus(UserId.ToSharedRef().Get()) != ELoginStatus::Type::LoggedIn) 
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to update current logged-in player's presence. Primary player is not logged-in."));
        return;
    }

    /* Only consider player is in a party if the party member is more than one.
     * This is because by the game design, the game always creates a party for the player.*/
    FString PartyStatus;
    if (UAccelByteWarsOnlineSessionBase* OnlineSession = GetOnlineSession())
    {
        PartyStatus = (OnlineSession->GetPartyMembers().Num() > 1) ? TEXT_PRESENCE_ACTIVITY_PARTY.ToString() : FString();
    }

    // Collect and construct presence status in "level - activity, party status" format.
    FString PresenceStatus = LevelStatus;
    if (!ActivityStatus.IsEmpty()) 
    {
        PresenceStatus += FString(" - ") + ActivityStatus;
    }
    if (!PartyStatus.IsEmpty())
    {
        PresenceStatus += (ActivityStatus.IsEmpty() ? FString(" - ") : FString(", ")) + PartyStatus;
    }

    // Set presence status.
    SetPresenceStatus(UserId, PresenceStatus);
}

void UPresenceEssentialsSubsystem::OnLevelLoaded()
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to update level status presence. World is not valid."));
        return;
    }

    const AAccelByteWarsGameState* GameState = Cast<AAccelByteWarsGameState>(World->GetGameState());

    /* On online multiplayer, the game state takes some time to be replicated after the world creation.
     * When the game state is replicated and initialized, try to update the level status.*/
    AAccelByteWarsGameState::OnInitialized.RemoveAll(this);
    if (!GameState) 
    {
        AAccelByteWarsGameState::OnInitialized.AddUObject(this, &ThisClass::OnLevelLoaded);
        return;
    }

    if (const AAccelByteWarsMainMenuGameState* MainMenuGameState = Cast<AAccelByteWarsMainMenuGameState>(GameState))
    {
        // Set level status to "In Main Menu".
        LevelStatus = TEXT_PRESENCE_LEVEL_MAINMENU.ToString();
        ActivityStatus = FString("");
    }
    else if (const AAccelByteWarsInGameGameState* InGameGameState = Cast<AAccelByteWarsInGameGameState>(GameState))
    {
        // Set level status to "In Match" and activity status to the current game mode type.
        LevelStatus = TEXT_PRESENCE_LEVEL_GAMEPLAY.ToString();
        ActivityStatus = FText::Format(TEXT_PRESENCE_ACTIVITY_GAMEPLAY, InGameGameState->GameSetup.DisplayName).ToString();
    }
    else
    {
        // Fallback, set status to unknown.
        LevelStatus = TEXT_PRESENCE_ACTIVITY_UNKNOWN.ToString();
        ActivityStatus = FString("");
    }

    UpdatePrimaryPlayerPresenceStatus();
}

void UPresenceEssentialsSubsystem::OnFriendListChange()
{
    FOnlineFriendsAccelBytePtr FriendsInterface = GetFriendsInterface();
    if (!FriendsInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query friends' presences. Friends interface is invalid."));
        return;
    }

    const FUniqueNetIdPtr UserId = GetPrimaryPlayerUserId();
    if (!UserId)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query friends' presences. Current logged-in player's User Id is invalid."));
        return;
    }

    // Get cached friend list.
    TArray<TSharedRef<FOnlineFriend>> OutFriendList;
    if (!FriendsInterface->GetFriendsList(0, TEXT(""), OutFriendList))
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query friends' presences. Cannot get cached friend list."));
        return;
    }
    if (OutFriendList.IsEmpty())
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query friends' presences. No friends found."));
        return;
    }

    // Collect their user ids.
    TArray<FUniqueNetIdRef> FriendIds;
    for (const TSharedRef<FOnlineFriend>& Friend : OutFriendList)
    {
        FriendIds.Add(Friend->GetUserId());
    }

    // Query friends' presences.
    UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Querying friends' presences."));
    BulkQueryPresence(UserId, FriendIds);
}

void UPresenceEssentialsSubsystem::OnBlockedPlayerListChange(int32 LocalUserNum, const FString& ListName)
{
    FOnlineFriendsAccelBytePtr FriendsInterface = GetFriendsInterface();
    if (!FriendsInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query blocked players' presences. Friends interface is invalid."));
        return;
    }

    const FUniqueNetIdPtr UserId = GetPrimaryPlayerUserId();
    if (!UserId)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query blocked player' presences. Current logged-in player's User Id is invalid."));
        return;
    }

    // Get cached blocked player list.
    TArray<TSharedRef<FOnlineBlockedPlayer>> OutBlockedPlayerList;
    if (!FriendsInterface->GetBlockedPlayers(UserId.ToSharedRef().Get(), OutBlockedPlayerList))
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query blocked players' presences. Cannot get cached blocked player list."));
        return;
    }
    if (OutBlockedPlayerList.IsEmpty())
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to query blocked players' presences. No blocked player found."));
        return;
    }

    // Collect their user ids.
    TArray<FUniqueNetIdRef> BlockedPlayerIds;
    for (const TSharedRef<FOnlineBlockedPlayer>& BlockedPlayer : OutBlockedPlayerList)
    {
        BlockedPlayerIds.Add(BlockedPlayer->GetUserId());
    }

    // Query blocked players' presences.
    UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Querying blocked players' presences."));
    BulkQueryPresence(UserId, BlockedPlayerIds);
}

void UPresenceEssentialsSubsystem::GetPresence(const FUniqueNetIdPtr UserId, const FOnPresenceTaskComplete& OnComplete)
{
    FOnlinePresenceAccelBytePtr PresenceInterface = GetPresenceInterface();
    if (!PresenceInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Cannot get presence. Presence interface is invalid."));
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    const FUniqueNetIdAccelByteUserPtr UserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(UserId);
    if (!UserABId)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Cannot get presence. User Id is invalid."));
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    // Try get the presence from cache.
    TSharedPtr<FOnlineUserPresence> OutPresence;
    PresenceInterface->GetCachedPresence(UserABId.ToSharedRef().Get(), OutPresence);
    if (TSharedPtr<FOnlineUserPresenceAccelByte> ABPresence = StaticCastSharedPtr<FOnlineUserPresenceAccelByte>(OutPresence))
    {
        UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Success to get presence for user: %s"), *UserABId->GetAccelByteId());
        OnComplete.ExecuteIfBound(true, ABPresence);
        return;
    }

    // If the presence is not available on cache, then query it.
    PresenceInterface->QueryPresence(
        UserABId.ToSharedRef().Get(),
        IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateUObject(this, &ThisClass::OnGetPresenceComplete, OnComplete));
}

void UPresenceEssentialsSubsystem::OnGetPresenceComplete(const FUniqueNetId& UserId, const bool bWasSuccessful, const FOnPresenceTaskComplete OnComplete)
{
    const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
    if (!UserABId->IsValid())
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Failed to get presence. User Id is invalid."));
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    FOnlinePresenceAccelBytePtr PresenceInterface = GetPresenceInterface();
    if (!PresenceInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Failed to get presence for user: %s. Presence interface is invalid."));
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    if (!bWasSuccessful)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Failed to get presence for user: %s. Operation failed."), *UserABId->GetAccelByteId());
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    TSharedPtr<FOnlineUserPresence> OutPresence;
    PresenceInterface->GetCachedPresence(UserABId.Get(), OutPresence);

    TSharedPtr<FOnlineUserPresenceAccelByte> ABPresence = StaticCastSharedPtr<FOnlineUserPresenceAccelByte>(OutPresence);
    if (!ABPresence)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Failed to get presence for user: %s. Invalid presence type."), *UserABId->GetAccelByteId());
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Success to get presence for user: %s"), *UserABId->GetAccelByteId());

    OnComplete.ExecuteIfBound(true, ABPresence);
}

void UPresenceEssentialsSubsystem::BulkQueryPresence(const FUniqueNetIdPtr UserId, const TArray<FUniqueNetIdRef>& UserIds)
{
    FUserIDPresenceMap CachedPresences;

    FOnlinePresenceAccelBytePtr PresenceInterface = GetPresenceInterface();
    if (!PresenceInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Cannot bulk query presence. Presence interface is invalid."));
        OnBulkQueryPresenceComplete(false, CachedPresences);
        return;
    }

    if (!UserId || UserIds.IsEmpty())
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Cannot bulk query presence. User Ids are invalid."));
        OnBulkQueryPresenceComplete(false, CachedPresences);
        return;
    }

    // Try to collect cached presences.
    for (const FUniqueNetIdRef& TargetUserId : UserIds)
    {
        const FUniqueNetIdAccelByteUserRef TargetUserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(TargetUserId);
        if (!TargetUserABId->IsValid())
        {
            return;
        }

        TSharedPtr<FOnlineUserPresence> OutPresence;
        PresenceInterface->GetCachedPresence(TargetUserABId.Get(), OutPresence);
        
        if (const TSharedPtr<FOnlineUserPresenceAccelByte> ABPresence = StaticCastSharedPtr<FOnlineUserPresenceAccelByte>(OutPresence))
        {
            CachedPresences.Add(TargetUserABId->GetAccelByteId(), ABPresence.ToSharedRef());
        }
        else
        {
            break;
        }
    }

    // The cached presences are complete, return the cached presence.
    if (CachedPresences.Num() == UserIds.Num())
    {
        OnBulkQueryPresenceComplete(true, CachedPresences);
        return;
    }

    // There are some missing cached presences, try to query.
    PresenceInterface->BulkQueryPresence(UserId.ToSharedRef().Get(), UserIds);
}

void UPresenceEssentialsSubsystem::OnBulkQueryPresenceComplete(const bool bWasSuccessful, const FUserIDPresenceMap& Presences)
{
    UE_LOG_PRESENCEESSENTIALS(Log, TEXT("%s to bulk query presences. Presences found: %d"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"), Presences.Num());

    OnBulkQueryPresenceCompleteDelegates.Broadcast(bWasSuccessful, Presences);
}

void UPresenceEssentialsSubsystem::SetPresenceStatus(const FUniqueNetIdPtr UserId, const FString& Status, const FOnPresenceTaskComplete& OnComplete)
{
    FOnlinePresenceAccelBytePtr PresenceInterface = GetPresenceInterface();
    if (!PresenceInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Cannot set presence status. Presence interface is invalid."));
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    if (!UserId)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Cannot set presence status. User Id is invalid."));
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    FOnlineUserPresenceStatus PresenceStatus;
    PresenceStatus.StatusStr = Status;
    PresenceStatus.State = EOnlinePresenceState::Type::Online;

    PresenceInterface->SetPresence(
        UserId.ToSharedRef().Get(),
        PresenceStatus,
        IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateUObject(this, &ThisClass::OnSetPresenceStatusComplete, OnComplete));
}

void UPresenceEssentialsSubsystem::OnSetPresenceStatusComplete(const FUniqueNetId& UserId, const bool bWasSuccessful, const FOnPresenceTaskComplete OnComplete)
{
    const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
    if (!UserABId->IsValid())
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Failed to set presence status. User Id is invalid."));
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    FOnlinePresenceAccelBytePtr PresenceInterface = GetPresenceInterface();
    if (!PresenceInterface)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Failed to set presence status for user: %s. Presence interface is invalid."), *UserABId->GetAccelByteId());
        OnComplete.ExecuteIfBound(false, nullptr);
        return;
    }

    TSharedPtr<FOnlineUserPresence> OutPresence;
    PresenceInterface->GetCachedPresence(UserABId.Get(), OutPresence);

    const TSharedPtr<FOnlineUserPresenceAccelByte> ABPresence = StaticCastSharedPtr<FOnlineUserPresenceAccelByte>(OutPresence);
    if (bWasSuccessful && ABPresence)
    {
        UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Success to set presence status for user: %s"), *UserABId->GetAccelByteId());
        OnComplete.ExecuteIfBound(true, ABPresence);
    }
    else
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Failed to set presence status for user: %s. Operation failed."), *UserABId->GetAccelByteId());
        OnComplete.ExecuteIfBound(false, nullptr);
    }
}

void UPresenceEssentialsSubsystem::OnPresenceReceived(const FUniqueNetId& UserId, const TSharedRef<FOnlineUserPresence>& Presence)
{
    const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(UserId.AsShared());
    if (!UserABId->IsValid()) 
    {
        return;
    }

    UE_LOG_PRESENCEESSENTIALS(Log, TEXT("Received presence update for user: %s"), *UserABId->GetAccelByteId());

    OnPresenceReceivedDelegates.Broadcast(UserABId.Get(), Presence);
}

FOnlinePresenceAccelBytePtr UPresenceEssentialsSubsystem::GetPresenceInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlinePresenceAccelByte>(Subsystem->GetPresenceInterface());
}

TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> UPresenceEssentialsSubsystem::GetFriendsInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
}

TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> UPresenceEssentialsSubsystem::GetIdentityInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

UAccelByteWarsOnlineSessionBase* UPresenceEssentialsSubsystem::GetOnlineSession() const
{
    if (!GetGameInstance()) 
    {
        return nullptr;
    }

    return Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
}

FUniqueNetIdPtr UPresenceEssentialsSubsystem::GetPrimaryPlayerUserId()
{
    if (!GetGameInstance())
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to get current logged in player's User Id. GameInstance is invalid."));
        return nullptr;
    }

    const APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to get current logged in player's User Id. PlayerController is invalid."));
        return nullptr;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to get current logged in player's User Id. LocalPlayer is invalid."));
        return nullptr;
    }

    return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
