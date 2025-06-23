// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PartyOnlineSession_Starter.h"

#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/Components/AccelByteWarsButtonBase.h"

#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

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

#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	if (GetOnPartyMembersChangeDelegates())
	{
		GetOnPartyMembersChangeDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Member, bool bJoined)
		{
			UpdatePartyGeneratedWidgets();
		});
	}
#else
	if (GetOnPartyMemberJoinedDelegates())
	{
		GetOnPartyMemberJoinedDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Member)
		{
			UpdatePartyGeneratedWidgets();
		});
	}
	if (GetOnPartyMemberLeftDelegates())
	{
		GetOnPartyMemberLeftDelegates()->AddWeakLambda(this, [this](FName SessionName, const FUniqueNetId& Member, EOnSessionParticipantLeftReason Reason)
		{
			UpdatePartyGeneratedWidgets();
		});
	}
#endif

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

	// Take local user ID reference from active widget.
	FUniqueNetIdPtr LocalUserABId = nullptr;
	if (UCommonActivatableWidget* ActiveWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this))
	{
		LocalUserABId = GetLocalPlayerUniqueNetId(ActiveWidget->GetOwningPlayer());
	}

	// Take current displayed friend ID.
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
#if UNREAL_ENGINE_VERSION_OLDER_THAN_5_2
	if (GetOnPartyMembersChangeDelegates())
	{
		GetOnPartyMembersChangeDelegates()->RemoveAll(this);
	}
#else
	if (GetOnPartyMemberJoinedDelegates())
	{
		GetOnPartyMemberJoinedDelegates()->RemoveAll(this);
	}
	if (GetOnPartyMemberLeftDelegates())
	{
		GetOnPartyMemberLeftDelegates()->RemoveAll(this);
	}
#endif

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