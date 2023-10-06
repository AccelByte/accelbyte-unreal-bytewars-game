// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/PartyEssentials/PartyOnlineSession_Starter.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"

#include "TutorialModules/Module-1/TutorialModuleOnlineUtility.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget.h"
#include "TutorialModules/Module-8/UI/FriendDetailsWidget_Starter.h"

void UPartyOnlineSession_Starter::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

    InitializePartyGeneratedWidgets();

    // TODO: Bind your party event delegates here.
}

void UPartyOnlineSession_Starter::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

    DeinitializePartyGeneratedWidgets();

    // TODO: Unbind your party event delegates here.
}

void UPartyOnlineSession_Starter::QueryUserInfo(const int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds, const FOnQueryUsersInfoComplete& OnComplete)
{
    // Safety
    if (!GetUserInt())
    {
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
        {
            OnComplete.ExecuteIfBound(false, {});
        }));
        return;
    }

    TArray<FUserOnlineAccountAccelByte*> UserInfo;
    if (RetrieveUserInfoCache(UserIds, UserInfo))
    {
        ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
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

void UPartyOnlineSession_Starter::OnQueryUserInfoComplete(int32 LocalUserNum, bool bSucceeded, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorMessage, const FOnQueryUsersInfoComplete& OnComplete)
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

void UPartyOnlineSession_Starter::OnLeavePartyToTriggerEvent(FName SessionName, bool bSucceeded, const TDelegate<void(bool bWasSuccessful)> OnComplete)
{
    // Abort if not a party session.
    if (SessionName != GetPredefinedSessionNameFromType(EAccelByteV2SessionType::PartySession))
    {
        OnComplete.ExecuteIfBound(false);
        return;
    }

    OnComplete.ExecuteIfBound(bSucceeded);
}

void UPartyOnlineSession_Starter::InitializePartyGeneratedWidgets()
{
    // Assign action button to invite player to the party.
    InviteToPartyButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_invite_to_party"));
    if (!ensure(InviteToPartyButtonMetadata))
    {
        return;
    }
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

    // Assign action button to kick player from the party.
    KickPlayerFromPartyButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_kick_from_party"));
    if (!ensure(KickPlayerFromPartyButtonMetadata))
    {
        return;
    }
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

    // Assign action button to promote party leader.
    PromotePartyLeaderButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_promote_party_leader"));
    if (!ensure(PromotePartyLeaderButtonMetadata))
    {
        return;
    }
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

    // On party member update events, update the generated widget.
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

void UPartyOnlineSession_Starter::UpdatePartyGeneratedWidgets()
{
    // Abort if not in a party session.
    if (!GetABSessionInt()->IsInPartySession())
    {
        return;
    }

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
                (bIsInParty && !bIsFriendInParty) ?
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

void UPartyOnlineSession_Starter::DeinitializePartyGeneratedWidgets()
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

FUniqueNetIdPtr UPartyOnlineSession_Starter::GetCurrentDisplayedFriendId()
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
    else if (const UFriendDetailsWidget_Starter* FriendDetailsWidget_Starter = Cast<UFriendDetailsWidget_Starter>(ParentWidget))
    {
        if (FriendDetailsWidget_Starter->GetCachedFriendData() &&
            FriendDetailsWidget_Starter->GetCachedFriendData()->UserId &&
            FriendDetailsWidget_Starter->GetCachedFriendData()->UserId.IsValid())
        {
            FriendUserId = FriendDetailsWidget_Starter->GetCachedFriendData()->UserId;
        }
    }

    if (FriendUserId == nullptr || !FriendUserId.IsValid())
    {
        return nullptr;
    }

    return FriendUserId.GetUniqueNetId();
}

void UPartyOnlineSession_Starter::OnInviteToPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& Invitee)
{
    // TODO: Send party invitation.
}

void UPartyOnlineSession_Starter::OnKickPlayerFromPartyButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& KickedPlayer)
{
    // TODO: Kick party member.
}

void UPartyOnlineSession_Starter::OnPromotePartyLeaderButtonClicked(const int32 LocalUserNum, const FUniqueNetIdPtr& NewLeader)
{
    // TODO: Promote a new party leader.
}

UPromptSubsystem* UPartyOnlineSession_Starter::GetPromptSubystem()
{
    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UPromptSubsystem>();
}

#pragma region "Party Essentials Module Function Definitions"
// TODO: Add your party essentials module function definitions here.
#pragma endregion