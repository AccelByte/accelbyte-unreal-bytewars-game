// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PartyOnlineSession.h"

#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"

void UPartyOnlineSession::RegisterOnlineDelegates()
{
    Super::RegisterOnlineDelegates();

    InitializePartyGeneratedWidgets();

    if (GetABSessionInt()) 
    {
        GetABSessionInt()->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::OnCreatePartyComplete);
        GetABSessionInt()->OnDestroySessionCompleteDelegates.AddUObject(this, &ThisClass::OnLeavePartyComplete);

        GetABSessionInt()->OnSendSessionInviteCompleteDelegates.AddUObject(this, &ThisClass::OnSendPartyInviteComplete);
        GetABSessionInt()->OnJoinSessionCompleteDelegates.AddUObject(this, &ThisClass::OnJoinPartyComplete);
        GetABSessionInt()->OnSessionInviteRejectedDelegates.AddUObject(this, &ThisClass::OnPartyInviteRejected);
        GetABSessionInt()->OnV2SessionInviteReceivedDelegates.AddUObject(this, &ThisClass::OnPartyInviteReceived);

        GetABSessionInt()->OnKickedFromSessionDelegates.AddUObject(this, &ThisClass::OnKickedFromParty);

        GetABSessionInt()->OnSessionParticipantsChangeDelegates.AddUObject(this, &ThisClass::OnPartyMembersChange);
        GetABSessionInt()->OnSessionUpdateReceivedDelegates.AddUObject(this, &ThisClass::OnPartySessionUpdateReceived);
    }

    if (GetABIdentityInt())
    {
        GetABIdentityInt()->OnConnectLobbyCompleteDelegates->AddUObject(this, &ThisClass::OnConnectLobbyComplete);
    }
}

void UPartyOnlineSession::ClearOnlineDelegates()
{
    Super::ClearOnlineDelegates();

    DeinitializePartyGeneratedWidgets();

    if (GetABSessionInt())
    {
        GetABSessionInt()->OnCreateSessionCompleteDelegates.RemoveAll(this);
        GetABSessionInt()->OnDestroySessionCompleteDelegates.RemoveAll(this);

        GetABSessionInt()->OnSendSessionInviteCompleteDelegates.RemoveAll(this);
        GetABSessionInt()->OnJoinSessionCompleteDelegates.RemoveAll(this);
        GetABSessionInt()->OnSessionInviteRejectedDelegates.RemoveAll(this);
        GetABSessionInt()->OnV2SessionInviteReceivedDelegates.RemoveAll(this);

        GetABSessionInt()->OnKickedFromSessionDelegates.RemoveAll(this);

        GetABSessionInt()->OnSessionParticipantsChangeDelegates.RemoveAll(this);
        GetABSessionInt()->OnSessionUpdateReceivedDelegates.RemoveAll(this);
    }

    if (GetABIdentityInt())
    {
        GetABIdentityInt()->OnConnectLobbyCompleteDelegates->RemoveAll(this);
    }
}

void UPartyOnlineSession::QueryUserInfo(const int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds, const FOnQueryUsersInfoComplete& OnComplete)
{
    // Safety
    if (!GetUserInt())
    {
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
        {
            OnComplete.ExecuteIfBound(false, {});
        }));
        return;
    }

    TArray<FUserOnlineAccountAccelByte*> UserInfo;
    if (RetrieveUserInfoCache(UserIds, UserInfo))
    {
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
        {
            OnComplete.ExecuteIfBound(true, UserInfo);
        }));
    }
    // Some data does not exist in cache, query everything
    else
    {
        // Bind delegate
        if (OnQueryUserInfoCompleteDelegateHandle.IsValid())
        {
            GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
            OnQueryUserInfoCompleteDelegateHandle.Reset();
        }
        OnQueryUserInfoCompleteDelegateHandle = GetUserInt()->OnQueryUserInfoCompleteDelegates->AddWeakLambda(
            this, [OnComplete, this](
                int32 LocalUserNum,
                bool bSucceeded,
                const TArray<FUniqueNetIdRef>& UserIds,
                const FString& ErrorMessage)
            {
                OnQueryUserInfoComplete(LocalUserNum, bSucceeded, UserIds, ErrorMessage, OnComplete);
            });

        if (!GetUserInt()->QueryUserInfo(LocalUserNum, UserIds))
        {
            OnQueryUserInfoComplete(LocalUserNum, false, UserIds, "", OnComplete);
        }
    }
}

void UPartyOnlineSession::OnQueryUserInfoComplete(int32 LocalUserNum, bool bSucceeded, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorMessage, const FOnQueryUsersInfoComplete& OnComplete)
{
    // reset delegate handle
    GetUserInt()->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
    OnQueryUserInfoCompleteDelegateHandle.Reset();

    if (bSucceeded)
    {
        // Cache the result.
        CacheUserInfo(LocalUserNum, UserIds);

        // Retrieve the result from cache.
        TArray<FUserOnlineAccountAccelByte*> OnlineUsers;
        RetrieveUserInfoCache(UserIds, OnlineUsers);

        // Only include valid users info only.
        OnlineUsers.RemoveAll([](const FUserOnlineAccountAccelByte* Temp)
        {
            return !Temp || !Temp->GetUserId()->IsValid();
        });

        UE_LOG_PARTYESSENTIALS(Log,
            TEXT("Queried users info: %d, found valid users info: %d"),
            UserIds.Num(), OnlineUsers.Num());

        OnComplete.ExecuteIfBound(true, OnlineUsers);
    }
    else
    {
        OnComplete.ExecuteIfBound(false, {});
    }
}

UPromptSubsystem* UPartyOnlineSession::GetPromptSubystem()
{
    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UPromptSubsystem>();
}

void UPartyOnlineSession::OnCreatePartyToInviteMember(FName SessionName, bool bWasSuccessful, const int32 LocalUserNum, const FUniqueNetIdPtr SenderId, const FUniqueNetIdPtr InviteeId)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    GetOnCreateSessionCompleteDelegates()->Remove(OnCreatePartyToInviteMemberDelegateHandle);

    if (!bWasSuccessful)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot send a party invitation. Failed to create a new party."));
        OnSendPartyInviteComplete(SenderId.ToSharedRef().Get(), SessionName, false, InviteeId.ToSharedRef().Get());
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Party created. Try sending a party invitation."));
        SendPartyInvite(LocalUserNum, InviteeId);
    }
}

void UPartyOnlineSession::OnLeavePartyToTriggerEvent(FName SessionName, bool bSucceeded, const TDelegate<void(bool bWasSuccessful)> OnComplete)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        OnComplete.ExecuteIfBound(false);
        return;
    }

    OnComplete.ExecuteIfBound(bSucceeded);
}

void UPartyOnlineSession::InitializePartyGeneratedWidgets()
{
    // Assign action button to invite player to the party.
    InviteToPartyButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_invite_to_party"));
    if (ensure(InviteToPartyButtonMetadata))
    {
        InviteToPartyButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
        {
            const UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
            if (!ParentWidget)
            {
                return;
            }

            const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();
            if (FriendUserId)
            {
                OnInviteToPartyButtonClicked(GetLocalUserNumFromPlayerController(ParentWidget->GetOwningPlayer()), FriendUserId);
            }
        });
        InviteToPartyButtonMetadata->OnWidgetGenerated.AddUObject(this, &ThisClass::UpdatePartyGeneratedWidgets);
    }

    // Assign action button to kick player from the party.
    KickPlayerFromPartyButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_kick_from_party"));
    if (ensure(KickPlayerFromPartyButtonMetadata))
    {
        KickPlayerFromPartyButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
        {
            const UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
            if (!ParentWidget)
            {
                return;
            }

            const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();
            if (FriendUserId)
            {
                OnKickPlayerFromPartyButtonClicked(GetLocalUserNumFromPlayerController(ParentWidget->GetOwningPlayer()), FriendUserId);
            }
        });
        KickPlayerFromPartyButtonMetadata->OnWidgetGenerated.AddUObject(this, &ThisClass::UpdatePartyGeneratedWidgets);
    }

    // Assign action button to promote party leader.
    PromotePartyLeaderButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_promote_party_leader"));
    if (ensure(PromotePartyLeaderButtonMetadata))
    {
        PromotePartyLeaderButtonMetadata->ButtonAction.AddWeakLambda(this, [this]()
        {
            const UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
            if (!ParentWidget)
            {
                return;
            }

            const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();
            if (FriendUserId)
            {
                OnPromotePartyLeaderButtonClicked(GetLocalUserNumFromPlayerController(ParentWidget->GetOwningPlayer()), FriendUserId);
            }
        });
        PromotePartyLeaderButtonMetadata->OnWidgetGenerated.AddUObject(this, &ThisClass::UpdatePartyGeneratedWidgets);
    }

    // On party update events, update the generated widget.
    if (GetOnCreatePartyCompleteDelegates())
    {
        GetOnCreatePartyCompleteDelegates()->AddWeakLambda(this, [this](FName SessionName, bool bWasSuccessful)
        {
            UpdatePartyGeneratedWidgets();
        });
    }
    if (GetOnLeavePartyCompleteDelegates())
    {
        GetOnLeavePartyCompleteDelegates()->AddWeakLambda(this, [this](FName SessionName, bool bWasSuccessful)
        {
            UpdatePartyGeneratedWidgets();
        });
    }
    if (GetOnPartyMembersChangeDelegates())
    {
        GetOnPartyMembersChangeDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Member, bool bJoined)
        {
            UpdatePartyGeneratedWidgets();
        });
    }
    if (GetOnPartySessionUpdateReceivedDelegates())
    {
        GetOnPartySessionUpdateReceivedDelegates()->AddWeakLambda(this, [this](FName SessionName)
        {
            UpdatePartyGeneratedWidgets();
        });
    }
}

void UPartyOnlineSession::UpdatePartyGeneratedWidgets()
{
    // Take local user id reference from active widget.
    FUniqueNetIdPtr LocalUserABId = nullptr;
    if (UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this))
    {
        LocalUserABId = GetLocalPlayerUniqueNetId(ActiveWidget->GetOwningPlayer());
    }

    // Take current displayed friend id.
    const FUniqueNetIdPtr FriendUserId = GetCurrentDisplayedFriendId();

    // Check party information.
    const bool bIsInParty = IsInParty(LocalUserABId);
    const bool bIsLeader = IsPartyLeader(LocalUserABId);
    const bool bIsFriendInParty = IsInParty(FriendUserId);

    // Display invite to party button if in a party.
    if (InviteToPartyButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button = 
            Cast<UAccelByteWarsButtonBase>(InviteToPartyButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(true);
            Button->SetVisibility(
                !bIsFriendInParty ?
                ESlateVisibility::Visible : 
                ESlateVisibility::Collapsed);
        }
    }

    // Display promote leader button if in a party and is the party leader.
    if (PromotePartyLeaderButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button =
            Cast<UAccelByteWarsButtonBase>(PromotePartyLeaderButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(true);
            Button->SetVisibility(
                (bIsInParty && bIsFriendInParty && bIsLeader) ? 
                ESlateVisibility::Visible : 
                ESlateVisibility::Collapsed);
        }
    }

    // Display kick player button if in a party and is the party leader.
    if (KickPlayerFromPartyButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button =
            Cast<UAccelByteWarsButtonBase>(KickPlayerFromPartyButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(true);
            Button->SetVisibility(
                (bIsInParty && bIsFriendInParty && bIsLeader) ? 
                ESlateVisibility::Visible : 
                ESlateVisibility::Collapsed);
        }
    }
}

void UPartyOnlineSession::DeinitializePartyGeneratedWidgets()
{
    // Unbind party action button delegates.
    if (InviteToPartyButtonMetadata)
    {
        InviteToPartyButtonMetadata->ButtonAction.RemoveAll(this);
        InviteToPartyButtonMetadata->OnWidgetGenerated.RemoveAll(this);
    }
    if (KickPlayerFromPartyButtonMetadata)
    {
        KickPlayerFromPartyButtonMetadata->ButtonAction.RemoveAll(this);
        KickPlayerFromPartyButtonMetadata->OnWidgetGenerated.RemoveAll(this);
    }
    if (PromotePartyLeaderButtonMetadata)
    {
        PromotePartyLeaderButtonMetadata->ButtonAction.RemoveAll(this);
        PromotePartyLeaderButtonMetadata->OnWidgetGenerated.RemoveAll(this);
    }

    // Unbind party event delegates.
    if (GetOnPartyMembersChangeDelegates())
    {
        GetOnPartyMembersChangeDelegates()->RemoveAll(this);
    }
    if (GetOnPartySessionUpdateReceivedDelegates())
    {
        GetOnPartySessionUpdateReceivedDelegates()->RemoveAll(this);
    }
}

FUniqueNetIdPtr UPartyOnlineSession::GetCurrentDisplayedFriendId()
{
    UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
    if (!ParentWidget)
    {
        return nullptr;
    }

    FUniqueNetIdRepl FriendUserId = nullptr;
    if (const UFriendDetailsWidget* FriendDetailsWidget = Cast<UFriendDetailsWidget>(ParentWidget))
    {
        if (FriendDetailsWidget->GetCachedFriendData() &&
            FriendDetailsWidget->GetCachedFriendData()->UserId &&
            FriendDetailsWidget->GetCachedFriendData()->UserId.IsValid())
        {
            FriendUserId = FriendDetailsWidget->GetCachedFriendData()->UserId;
        }
    }

    if (FriendUserId == nullptr || !FriendUserId.IsValid())
    {
        return nullptr;
    }

    return FriendUserId.GetUniqueNetId();
}

void UPartyOnlineSession::OnInviteToPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee)
{
    // Disable the button to avoid spamming.
    if (InviteToPartyButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button = 
            Cast<UAccelByteWarsButtonBase>(InviteToPartyButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(false);
        }
    }

    SendPartyInvite(LocalUserNum, Invitee);
}

void UPartyOnlineSession::OnKickPlayerFromPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer)
{
    // Disable the button to avoid spamming.
    if (KickPlayerFromPartyButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button =
            Cast<UAccelByteWarsButtonBase>(KickPlayerFromPartyButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(false);
        }
    }

    KickPlayerFromParty(LocalUserNum, KickedPlayer);
}

void UPartyOnlineSession::OnPromotePartyLeaderButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader)
{
    // Disable the button to avoid spamming.
    if (PromotePartyLeaderButtonMetadata)
    {
        if (UAccelByteWarsButtonBase* Button =
            Cast<UAccelByteWarsButtonBase>(PromotePartyLeaderButtonMetadata->GenerateWidgetRef))
        {
            Button->SetIsInteractionEnabled(false);
        }
    }

    PromotePartyLeader(LocalUserNum, NewLeader);
}

#pragma region "Party Essentials Module Function Definitions"

TArray<FUniqueNetIdRef> UPartyOnlineSession::GetPartyMembers()
{
    if (GetABSessionInt())
    {
        const FNamedOnlineSession* PartySession = GetABSessionInt()->GetNamedSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession));
        if (PartySession)
        {
            return PartySession->RegisteredPlayers;
        }
    }

    return TArray<FUniqueNetIdRef>();
}

FUniqueNetIdPtr UPartyOnlineSession::GetPartyLeader()
{
    if (GetABSessionInt())
    {
        const FNamedOnlineSession* PartySession = GetABSessionInt()->GetNamedSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession));
        if (PartySession)
        {
            const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(PartySession->SessionInfo);
            if (!SessionInfo)
            {
                return nullptr;
            }

            return GetABSessionInt()->GetSessionLeaderId(PartySession);
        }
    }

    return nullptr;
}

bool UPartyOnlineSession::IsInParty(const FUniqueNetIdPtr UserId)
{
    if (!UserId)
    {
        return false;
    }

    const TPartyMemberArray Members = GetPartyMembers();
    for (const auto& Member : Members)
    {
        if (!Member.Get().IsValid())
        {
            continue;
        }

        if (Member.Get() == UserId.ToSharedRef().Get())
        {
            return true;
        }
    }

    return false;
}

bool UPartyOnlineSession::IsPartyLeader(const FUniqueNetIdPtr UserId)
{
    return GetPartyLeader() && UserId && UserId.ToSharedRef().Get() == GetPartyLeader().ToSharedRef().Get();
}

void UPartyOnlineSession::CreateParty(const int32 LocalUserNum)
{
    const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);

    // Safety.
    if (!GetABSessionInt())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot create a party. Session Interface is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
        {
            OnCreatePartyComplete(SessionName, false);
        }));
        return;
    }

    // Always create a new party. Thus, leave any left-over party session first.
    if (GetABSessionInt()->IsInPartySession())
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Party found. Leave old party before creating a new one."));

        if (OnLeaveSessionForTriggerDelegateHandle.IsValid())
        {
            GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);
            OnLeaveSessionForTriggerDelegateHandle.Reset();
        }

        OnLeaveSessionForTriggerDelegateHandle = GetOnLeaveSessionCompleteDelegates()->AddUObject(
            this,
            &ThisClass::OnLeavePartyToTriggerEvent,
            TDelegate<void(bool)>::CreateWeakLambda(this, [this, LocalUserNum, SessionName](bool bWasSuccessful)
            {
                GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);

                if (bWasSuccessful)
                {
                    UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to leave old party destroyed. Try creating a new party."));
                    CreateParty(LocalUserNum);
                }
                else
                {
                    UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot create a new party. Failed to leave old party."));
                    ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
                    {
                        OnCreatePartyComplete(SessionName, false);
                    }));
                }
            }
        ));

        LeaveSession(SessionName);
        return;
    }

    // Create a new party session.
    UE_LOG_PARTYESSENTIALS(Log, TEXT("Create a new party."));
    CreateSession(
        LocalUserNum,
        SessionName,
        FOnlineSessionSettings(),
        EAccelByteV2SessionType::PartySession,
        PartySessionTemplate);
}

void UPartyOnlineSession::LeaveParty(const int32 LocalUserNum)
{
    const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);

    if (!GetABSessionInt())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot leave a party. Session Interface is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
        {
            OnLeavePartyComplete(SessionName, false);
        }));
        return;
    }

    if (!GetABSessionInt()->IsInPartySession())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot leave a party. Not in any party."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
        {
            OnLeavePartyComplete(SessionName, false);
        }));
        return;
    }

    // Leave party.
    UE_LOG_PARTYESSENTIALS(Log, TEXT("Leave party."));
    LeaveSession(SessionName);
}

void UPartyOnlineSession::SendPartyInvite(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee)
{
    if (!GetABSessionInt())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot send a party invitation. Session Interface is not valid."));
        return;
    }

    const APlayerController* SenderPC = GetPlayerControllerByLocalUserNum(LocalUserNum);
    if (!SenderPC)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot send a party invitation. Sender's PlayerController is not valid."));
        return;
    }

    const FUniqueNetIdPtr SenderId = GetLocalPlayerUniqueNetId(SenderPC);
    if (!SenderId)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot send a party invitation. Sender's NetId is not valid."));
        return;
    }

    const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);
    if (!Invitee)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot send a party invitation. Invitee's NetId is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SenderId, SessionName, Invitee]()
        {
            OnSendPartyInviteComplete(SenderId.ToSharedRef().Get(), SessionName, false, Invitee.ToSharedRef().Get());
        }));
        return;
    }

    // Create a new party first before inviting.
    if (!GetABSessionInt()->IsInPartySession())
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Not in a party session. Creating a new party before sending a party invitation."));

        if (OnCreatePartyToInviteMemberDelegateHandle.IsValid())
        {
            GetOnCreateSessionCompleteDelegates()->Remove(OnCreatePartyToInviteMemberDelegateHandle);
            OnCreatePartyToInviteMemberDelegateHandle.Reset();
        }

        OnCreatePartyToInviteMemberDelegateHandle = GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreatePartyToInviteMember, LocalUserNum, SenderId, Invitee);

        CreateParty(LocalUserNum);
        return;
    }

    // Send party invitation.
    UE_LOG_PARTYESSENTIALS(Log, TEXT("Send party invitation."));
    GetABSessionInt()->SendSessionInviteToFriend(
        SenderId.ToSharedRef().Get(),
        SessionName,
        Invitee.ToSharedRef().Get());
}

void UPartyOnlineSession::JoinParty(const int32 LocalUserNum, const FOnlineSessionSearchResult& PartySessionResult)
{
    const FName SessionName = GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession);

    if (!GetABSessionInt())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot join a party. Session Interface is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
        {
            OnJoinPartyComplete(SessionName, EOnJoinSessionCompleteResult::Type::UnknownError);
        }));
        return;
    }

    // Always leave any party before joining a new party.
    if (GetABSessionInt()->IsInPartySession())
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Party found. Leave old party before joining a new one."));

        if (OnLeaveSessionForTriggerDelegateHandle.IsValid())
        {
            GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);
            OnLeaveSessionForTriggerDelegateHandle.Reset();
        }

        OnLeaveSessionForTriggerDelegateHandle = GetOnLeaveSessionCompleteDelegates()->AddUObject(
            this,
            &ThisClass::OnLeavePartyToTriggerEvent,
            TDelegate<void(bool)>::CreateWeakLambda(this, [this, LocalUserNum, PartySessionResult, SessionName](bool bWasSuccessful)
            {
                GetOnLeaveSessionCompleteDelegates()->Remove(OnLeaveSessionForTriggerDelegateHandle);

                if (bWasSuccessful)
                {
                    UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to leave old party. Try to joining a new party."));

                    JoinParty(LocalUserNum, PartySessionResult);
                }
                else
                {
                    UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot joining a new party. Failed to leave old party."));

                    ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, SessionName]()
                    {
                        OnJoinPartyComplete(SessionName, EOnJoinSessionCompleteResult::Type::UnknownError);
                    }));
                }
            }
        ));

        LeaveSession(SessionName);
        return;
    }

    // Join a new party.
    UE_LOG_PARTYESSENTIALS(Log, TEXT("Join a new party."));
    JoinSession(LocalUserNum, SessionName, PartySessionResult);
}

void UPartyOnlineSession::RejectPartyInvite(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite)
{
    if (!GetABSessionInt())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot reject a party invitation. Session Interface is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            OnRejectPartyInviteComplete(false);
        }));
        return;
    }

    const APlayerController* RejecterPC = GetPlayerControllerByLocalUserNum(LocalUserNum);
    if (!RejecterPC)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot reject a party invitation. Rejecter's PlayerController is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            OnRejectPartyInviteComplete(false);
        }));
        return;
    }

    const FUniqueNetIdPtr RejecterId = GetLocalPlayerUniqueNetId(RejecterPC);
    if (!RejecterId)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot reject a party invitation. Rejecter's NetId is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            OnRejectPartyInviteComplete(false);
        }));
        return;
    }

    UE_LOG_PARTYESSENTIALS(Log, TEXT("Reject party invitation."));
    GetABSessionInt()->RejectInvite(
        RejecterId.ToSharedRef().Get(),
        PartyInvite,
        FOnRejectSessionInviteComplete::CreateUObject(this, &ThisClass::OnRejectPartyInviteComplete));
}

void UPartyOnlineSession::KickPlayerFromParty(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer)
{
    if (!GetABSessionInt())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot kick a player from the party. Session Interface is not valid."));
        return;
    }

    if (!KickedPlayer)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot kick a player from the party. KickedPlayer's NetId is not valid."));
        return;
    }

    const APlayerController* PC = GetPlayerControllerByLocalUserNum(LocalUserNum);
    if (!PC)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot kick a player from the party. Kicker's PlayerController is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, KickedPlayer]()
        {
            OnKickPlayerFromPartyComplete(false, KickedPlayer.ToSharedRef().Get());
        }));
        return;
    }

    const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
    if (!PlayerNetId)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot kick a player from the party. Kicker's NetId is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, KickedPlayer]()
        {
            OnKickPlayerFromPartyComplete(false, KickedPlayer.ToSharedRef().Get());
        }));
        return;
    }

    UE_LOG_PARTYESSENTIALS(Log, TEXT("Kick party member."));
    GetABSessionInt()->KickPlayer(
        PlayerNetId.ToSharedRef().Get(),
        GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession),
        KickedPlayer.ToSharedRef().Get(),
        FOnKickPlayerComplete::CreateUObject(this, &ThisClass::OnKickPlayerFromPartyComplete));
}

void UPartyOnlineSession::PromotePartyLeader(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader)
{
    if (!GetABSessionInt())
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot promote new party leader. Session Interface is not valid."));
        return;
    }

    if (!NewLeader)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot promote new party leader. New Leader NetId is not valid."));
        return;
    }

    const APlayerController* PC = GetPlayerControllerByLocalUserNum(LocalUserNum);
    if (!PC)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot promote new party leader. Promoter's PlayerController is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, NewLeader]()
        {
            OnPromotePartyLeaderComplete(NewLeader.ToSharedRef().Get(), FOnlineError(false));
        }));
        return;
    }

    const FUniqueNetIdPtr PlayerNetId = GetLocalPlayerUniqueNetId(PC);
    if (!PlayerNetId)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot promote new party leader. Promoter's NetId is not valid."));
        ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, NewLeader]()
        {
            OnPromotePartyLeaderComplete(NewLeader.ToSharedRef().Get(), FOnlineError(false));
        }));
        return;
    }

    UE_LOG_PARTYESSENTIALS(Log, TEXT("Promote a new party leader."));
    GetABSessionInt()->PromotePartySessionLeader(
        PlayerNetId.ToSharedRef().Get(),
        GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession),
        NewLeader.ToSharedRef().Get(),
        FOnPromotePartySessionLeaderComplete::CreateUObject(this, &ThisClass::OnPromotePartyLeaderComplete));
}

void UPartyOnlineSession::OnCreatePartyComplete(FName SessionName, bool bSucceeded)
{
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    if (bSucceeded)
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to create a party"));
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to create a party"));
    }

    // Cache the party leader.
    LastPartyLeader = GetPartyLeader();

    // Reset the party member status cache.
    PartyMemberStatus.Empty();

    OnCreatePartyCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UPartyOnlineSession::OnLeavePartyComplete(FName SessionName, bool bSucceeded)
{
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    if (bSucceeded)
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to leave a party"));
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to leave a party"));
    }

    OnLeavePartyCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UPartyOnlineSession::OnSendPartyInviteComplete(const FUniqueNetId& Sender, FName SessionName, bool bWasSuccessful, const FUniqueNetId& Invitee)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    const FUniqueNetIdAccelByteUserRef InviteeABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Invitee.AsShared());
    if (bWasSuccessful)
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to send party invitation to %s"),
            InviteeABId->IsValid() ? *InviteeABId->GetAccelByteId() : TEXT("Unknown"));
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to send party invitation to %s"),
            InviteeABId->IsValid() ? *InviteeABId->GetAccelByteId() : TEXT("Unknown"));
    }

    // Display push notification.
    if (GetPromptSubystem())
    {
        GetPromptSubystem()->PushNotification(bWasSuccessful ? SUCCESS_SEND_PARTY_INVITE : FAILED_SEND_PARTY_INVITE);
    }

    UpdatePartyGeneratedWidgets();

    OnSendPartyInviteCompleteDelegates.Broadcast(Sender, SessionName, bWasSuccessful, Invitee);
}

void UPartyOnlineSession::OnJoinPartyComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    if (Result == EOnJoinSessionCompleteResult::Type::Success)
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to join a party"));
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to join a party"));
    }

    // Cache the party leader.
    LastPartyLeader = GetPartyLeader();

    // Reset the party member status cache.
    PartyMemberStatus.Empty();

    OnJoinPartyCompleteDelegates.Broadcast(SessionName, Result);
}

void UPartyOnlineSession::OnRejectPartyInviteComplete(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to reject party invitation"));
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to reject party invitation"));
    }

    OnRejectPartyInviteCompleteDelegate.ExecuteIfBound(bWasSuccessful);
    OnRejectPartyInviteCompleteDelegate.Unbind();
}

void UPartyOnlineSession::OnPartyInviteRejected(FName SessionName, const FUniqueNetId& RejecterId)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    const FUniqueNetIdAccelByteUserRef RejecterABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(RejecterId.AsShared());
    UE_LOG_PARTYESSENTIALS(Log, TEXT("Party invitation is rejected by %s"),
        RejecterABId->IsValid() ? *RejecterABId->GetAccelByteId() : TEXT("Unknown"));

    // Display push notification to show who rejected the invitation.
    QueryUserInfo(0, TPartyMemberArray{ RejecterABId },
        FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, RejecterABId](const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
        {
            if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
            {
                return;
            }

            FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

            const FText NotifMessage = FText::Format(PARTY_INVITE_REJECTED_MESSAGE, FText::FromString(
                MemberInfo.GetDisplayName().IsEmpty() ?
                UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(RejecterABId.Get()) :
                MemberInfo.GetDisplayName()
            ));

            FString AvatarURL;
            MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

            GetPromptSubystem()->PushNotification(NotifMessage, AvatarURL, true);
        }
    ));

    OnPartyInviteRejectedDelegates.Broadcast(SessionName, RejecterId);
}

void UPartyOnlineSession::OnPartyInviteReceived(const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FOnlineSessionInviteAccelByte& PartyInvite)
{
    // Abort if not a party session.
    if (UserId == FromId || PartyInvite.SessionType != EAccelByteV2SessionType::PartySession)
    {
        return;
    }

    const APlayerController* PC = GetPlayerControllerByUniqueNetId(UserId.AsShared());
    if (!PC)
    {
        return;
    }

    const FUniqueNetIdAccelByteUserRef SenderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(FromId.AsShared());
    UE_LOG_PARTYESSENTIALS(Log, TEXT("Receives party invitation from %s"),
        SenderABId->IsValid() ? *SenderABId->GetAccelByteId() : TEXT("Unknown"));

    const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

    // Display push notification to allow player to accept/reject the party invitation.
    QueryUserInfo(0, TPartyMemberArray{ SenderABId },
        FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, LocalUserNum, SenderABId, PartyInvite]
        (const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
        {
            if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
            {
                return;
            }

            FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

            const FText NotifMessage = FText::Format(PARTY_INVITE_RECEIVED_MESSAGE, FText::FromString(
                MemberInfo.GetDisplayName().IsEmpty() ?
                UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(SenderABId.Get()) :
                MemberInfo.GetDisplayName()
            ));

            FString AvatarURL;
            MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

            GetPromptSubystem()->PushNotification(
                NotifMessage,
                AvatarURL,
                true,
                ACCEPT_PARTY_INVITE_MESSAGE,
                REJECT_PARTY_INVITE_MESSAGE,
                FText::GetEmpty(),
                FPushNotificationDelegate::CreateWeakLambda(this, [this, LocalUserNum, PartyInvite](EPushNotificationActionResult ActionButtonResult)
                {
                    switch (ActionButtonResult)
                    {
                    // Show accept party invitation confirmation.
                    case EPushNotificationActionResult::Button1:
                        DisplayJoinPartyConfirmation(LocalUserNum, PartyInvite);
                        break;
                    // Reject party invitation.
                    case EPushNotificationActionResult::Button2:
                        RejectPartyInvite(LocalUserNum, PartyInvite);
                        break;
                    }
                }
            ));
        }
    ));

    OnPartyInviteReceivedDelegate.ExecuteIfBound(UserId, FromId, PartyInvite);
    OnPartyInviteReceivedDelegate.Unbind();
}

void UPartyOnlineSession::DisplayJoinPartyConfirmation(const int32 LocalUserNum, const FOnlineSessionInviteAccelByte& PartyInvite)
{
    // Join the party if not in any party yet.
    if (!GetABSessionInt()->IsInPartySession() || GetPartyMembers().Num() <= 1)
    {
        JoinParty(LocalUserNum, PartyInvite.Session);
        return;
    }

    // Show confirmation to leave current party and join the new party.
    GetPromptSubystem()->ShowDialoguePopUp(
        PARTY_POPUP_MESSAGE,
        JOIN_NEW_PARTY_CONFIRMATION_MESSAGE,
        EPopUpType::ConfirmationYesNo,
        FPopUpResultDelegate::CreateWeakLambda(this, [this, LocalUserNum, PartyInvite](EPopUpResult Result)
        {
            switch (Result)
            {
            case EPopUpResult::Confirmed:
                // If confirmed, join the new party.
                JoinParty(LocalUserNum, PartyInvite.Session);
                break;
            case EPopUpResult::Declined:
                // If declined, reject the party invitation.
                RejectPartyInvite(LocalUserNum, PartyInvite);
                break;
            }
        }
    ));
}

void UPartyOnlineSession::OnKickPlayerFromPartyComplete(bool bWasSuccessful, const FUniqueNetId& KickedPlayer)
{
    const FUniqueNetIdAccelByteUserRef KickedPlayerABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(KickedPlayer.AsShared());
    if (bWasSuccessful)
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to kick %s from the party."),
            KickedPlayerABId->IsValid() ? *KickedPlayerABId->GetAccelByteId() : TEXT("Unknown"));
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to kick %s from the party."),
            KickedPlayerABId->IsValid() ? *KickedPlayerABId->GetAccelByteId() : TEXT("Unknown"));
    }

    OnKickPlayerFromPartyCompleteDelegate.ExecuteIfBound(bWasSuccessful, KickedPlayer);
    OnKickPlayerFromPartyCompleteDelegate.Unbind();
}

void UPartyOnlineSession::OnKickedFromParty(FName SessionName)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    UE_LOG_PARTYESSENTIALS(Log, TEXT("Current logged player is kicked from the party"));

    // Display push notification.
    if (GetPromptSubystem())
    {
        GetPromptSubystem()->PushNotification(KICKED_FROM_PARTY_MESSAGE);
    }

    OnKickedFromPartyDelegates.Broadcast(SessionName);
}

void UPartyOnlineSession::OnPromotePartyLeaderComplete(const FUniqueNetId& NewLeader, const FOnlineError& Result)
{
    const FUniqueNetIdAccelByteUserRef NewLeaderABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(NewLeader.AsShared());
    if (Result.bSucceeded)
    {
        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to promote %s as the new party leader."),
            NewLeaderABId->IsValid() ? *NewLeaderABId->GetAccelByteId() : TEXT("Unknown"));
    }
    else
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to promote %s as the new party leader."),
            NewLeaderABId->IsValid() ? *NewLeaderABId->GetAccelByteId() : TEXT("Unknown"));
    }

    OnPromotePartyLeaderCompleteDelegate.ExecuteIfBound(NewLeader, Result);
    OnPromotePartyLeaderCompleteDelegate.Unbind();
}

void UPartyOnlineSession::DisplayCurrentPartyLeader()
{
    // Abort if the party leader is the same.
    if (LastPartyLeader && IsPartyLeader(LastPartyLeader))
    {
        return;
    }

    LastPartyLeader = GetPartyLeader();
    const FUniqueNetIdAccelByteUserPtr LeaderABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(LastPartyLeader);

    // Query party leader information and then display a notification.
    QueryUserInfo(0, TPartyMemberArray{ LeaderABId.ToSharedRef() },
        FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, LeaderABId]
        (const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
        {
            if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
            {
                return;
            }

            FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

            const FText NotifMessage = FText::Format(PARTY_NEW_LEADER_MESSAGE, FText::FromString(
                MemberInfo.GetDisplayName().IsEmpty() ?
                UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(LeaderABId.ToSharedRef().Get()) :
                MemberInfo.GetDisplayName()
            ));

            FString AvatarURL;
            MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

            GetPromptSubystem()->PushNotification(NotifMessage, AvatarURL, true);
        }
    ));
}

void UPartyOnlineSession::OnPartyMembersChange(FName SessionName, const FUniqueNetId& Member, bool bJoined)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    // Store status whether the member is the current logged-in player.
    FUniqueNetIdPtr UserId = nullptr;
    if (GetIdentityInt())
    {
        UserId = GetIdentityInt()->GetUniquePlayerId(0);
    }
    const bool bIsMemberTheLoggedInPlayer = UserId && UserId.ToSharedRef().Get() == Member;

    const FUniqueNetIdAccelByteUserRef MemberABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(Member.AsShared());
    const FString MemberABIdStr = MemberABId->GetAccelByteId();

    /* Since this event could be called multiple times, we cache the party member status.
     * This cache is used to execute the following functionalities only when the party member status is changed (not the same status).*/
    if (PartyMemberStatus.Contains(MemberABIdStr))
    {
        if (PartyMemberStatus[MemberABIdStr] == bJoined)
        {
            // Abort if the status is the same.
            return;
        }
        PartyMemberStatus[MemberABIdStr] = bJoined;
    }
    if (!PartyMemberStatus.Contains(MemberABIdStr))
    {
        PartyMemberStatus.Add(MemberABIdStr, bJoined);
    }

    UE_LOG_PARTYESSENTIALS(Log, TEXT("Party participant %s %s to/from the party"),
        MemberABId->IsValid() ? *MemberABId->GetAccelByteId() : TEXT("Unknown"),
        bJoined ? TEXT("joined") : TEXT("left"));

    // Query member information then display a push notification to show who joined/left the party.
    if (!bIsMemberTheLoggedInPlayer)
    {
        QueryUserInfo(0, TPartyMemberArray{ MemberABId },
            FOnQueryUsersInfoComplete::CreateWeakLambda(this, [this, MemberABId, bJoined]
            (const bool bSucceeded, const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
            {
                if (UsersInfo.IsEmpty() || !UsersInfo[0] || !GetPromptSubystem())
                {
                    return;
                }

                FUserOnlineAccountAccelByte MemberInfo = *UsersInfo[0];

                const FString MemberDisplayName = MemberInfo.GetDisplayName().IsEmpty() ?
                    UTutorialModuleOnlineUtility::GetUserDefaultDisplayName(MemberABId.Get()) :
                    MemberInfo.GetDisplayName();

                const FText NotifMessage = bJoined ?
                    FText::Format(PARTY_MEMBER_JOINED_MESSAGE, FText::FromString(MemberDisplayName)) :
                    FText::Format(PARTY_MEMBER_LEFT_MESSAGE, FText::FromString(MemberDisplayName));

                FString AvatarURL;
                MemberInfo.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);

                GetPromptSubystem()->PushNotification(NotifMessage, AvatarURL, true);
            }
        ));
    }

    // Show notification if a new party leader is set.
    DisplayCurrentPartyLeader();

    OnPartyMembersChangeDelegates.Broadcast(SessionName, Member, bJoined);
}

void UPartyOnlineSession::OnPartySessionUpdateReceived(FName SessionName)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        return;
    }

    UE_LOG_PARTYESSENTIALS(Log, TEXT("Party session is updated"));

    // Show notification if a new party leader is set.
    DisplayCurrentPartyLeader();

    OnPartySessionUpdateReceivedDelegates.Broadcast(SessionName);
}

void UPartyOnlineSession::OnConnectLobbyComplete(int32 LocalUserNum, bool bSucceeded, const FUniqueNetId& UserId, const FString& Error)
{
    if (!bSucceeded)
    {
        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Cannot restore and leave restored party session. Failed to connect to lobby. Error: %s."), *Error);
        return;
    }

    // Restore and leave old party session.
    GetABSessionInt()->RestoreActiveSessions(
        UserId,
        FOnRestoreActiveSessionsComplete::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlineError& Result)
        {
            // Abort if failed to restore party sessions.
            if (!Result.bSucceeded)
            {
                UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to restore party session. Error: %s"), *Result.ErrorMessage.ToString());
                return;
            }

            // Safety.
            if (!GetABSessionInt())
            {
                UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to restore party session. Session Interface is not valid."));
                return;
            }

            const TArray<FOnlineRestoredSessionAccelByte> RestoredParties = GetABSessionInt()->GetAllRestoredPartySessions();

            // If no restored party session, do nothing.
            if (RestoredParties.IsEmpty())
            {
                UE_LOG_PARTYESSENTIALS(Log, TEXT("No restored party session found. Do nothing."));
                return;
            }

            // Leave the first restored active party session.
            UE_LOG_PARTYESSENTIALS(Log, TEXT("Restored party session found. Leave the restored party session."));
            GetABSessionInt()->LeaveRestoredSession(
                LocalUserId,
                RestoredParties[0],
                FOnLeaveSessionComplete::CreateWeakLambda(this, [](bool bWasSuccessful, FString SessionId)
                {
                    if (bWasSuccessful)
                    {
                        UE_LOG_PARTYESSENTIALS(Log, TEXT("Success to leave restored party session."));
                    }
                    else
                    {
                        UE_LOG_PARTYESSENTIALS(Warning, TEXT("Failed to leave restored party session."));
                    }
                }
            ));
        })
    );
}

#pragma endregion