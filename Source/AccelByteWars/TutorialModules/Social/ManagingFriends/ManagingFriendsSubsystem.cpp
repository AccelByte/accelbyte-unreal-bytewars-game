// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ManagingFriendsSubsystem.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"
#include "Social/FriendsEssentials/FriendsSubsystem.h"

void UManagingFriendsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get Online Subsystem and make sure it's valid.
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return;
	}

	// Grab the reference of AccelByte User Interface and make sure it's valid.
	UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
	if (!ensure(UserInterface.IsValid()))
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("User Interface is not valid."));
		return;
	}

	// Grab the reference of AccelByte Friends Interface and make sure it's valid.
	FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
	if (!ensure(FriendsInterface.IsValid()))
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Friends Interface is not valid."));
		return;
	}

	// Grab prompt subsystem.
	PromptSubsystem = GetGameInstance()->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);

	FriendsSubsystem = GetGameInstance()->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);
}

void UManagingFriendsSubsystem::Deinitialize()
{
	Super::Deinitialize();

	// Clear on blocked players changed delegate.
	for (auto& DelegateHandle : OnBlockedPlayersChangeDelegateHandles)
	{
		FriendsInterface->ClearOnFriendsChangeDelegate_Handle(DelegateHandle.Key, DelegateHandle.Value);
	}
}

void UManagingFriendsSubsystem::BindGeneratedWidgetButtonsAction(const APlayerController* PlayerController, UFriendData* PlayerData)
{
	auto OnGeneratedButtonActionComplete = [this](const APlayerController* PlayerController, UFriendData* PlayerData)
	{
		if(PlayerData->DataSource != EDataSource::User)
		{
			UpdateFriendStatus(PlayerController, PlayerData);
			SetGeneratedWidgetButtonsVisibility(PlayerData);
		}
		else
		{
			const TWeakObjectPtr<AAccelByteWarsMainMenuGameState> MainMenuGameState = 
				MakeWeakObjectPtr<AAccelByteWarsMainMenuGameState>(Cast<AAccelByteWarsMainMenuGameState>(GetWorld()->GetGameState()));
			const EBaseUIStackType StackType = MainMenuGameState.IsValid() ? EBaseUIStackType::Menu : EBaseUIStackType::InGameMenu;
			UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(StackType, this);
			
			if (!ParentWidget)
			{
				return;
			}
			
			// Close the friend details widget if successful.
			ParentWidget->DeactivateWidget();
		}
	};
	// Assign action button to unfriend.
	FTutorialModuleGeneratedWidget* UnfriendButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unfriend"), AssociateTutorialModule->GeneratedWidgets);
	ensure(UnfriendButtonMetadata);
	UnfriendButtonMetadata->ButtonAction.AddWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete]()
	{
		const FUniqueNetIdRepl FriendUserId = PlayerData->UserId;
		ensure(FriendUserId.IsValid());

		OnUnfriendButtonClicked(
			PlayerController,
			FriendUserId,
			FOnUnfriendComplete::CreateWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete](bool bWasSuccessful, const FString& ErrorMessage)
			{
				if(bWasSuccessful)
				{
					OnGeneratedButtonActionComplete(PlayerController, PlayerData);
				}				
			}
		));
	});

	// Assign action button to block a player.
	FTutorialModuleGeneratedWidget* BlockPlayerButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_block_player"), AssociateTutorialModule->GeneratedWidgets);
	ensure(BlockPlayerButtonMetadata);
	BlockPlayerButtonMetadata->ButtonAction.AddWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete]()
	{
		const FUniqueNetIdRepl BlockedPlayerUserId = PlayerData->UserId;
		ensure(BlockedPlayerUserId.IsValid());

		OnBlockButtonClicked(
			PlayerController,
			BlockedPlayerUserId,
			FOnBlockPlayerComplete::CreateWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete](bool bWasSuccessful, const FString& ErrorMessage)
			{
				if (bWasSuccessful)
				{
					OnGeneratedButtonActionComplete(PlayerController, PlayerData);
				}
			}
		));
	});
	
	// Assign action button to unblock a player.
	FTutorialModuleGeneratedWidget* UnblockPlayerButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unblock"), AssociateTutorialModule->GeneratedWidgets);
	ensure(UnblockPlayerButtonMetadata);
	UnblockPlayerButtonMetadata->ButtonAction.AddWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete]()
	{
		const FUniqueNetIdRepl UnblockedPlayerUserId = PlayerData->UserId;
		ensure(UnblockedPlayerUserId.IsValid());

		OnUnblockButtonClicked(
			PlayerController,
			UnblockedPlayerUserId,
			FOnUnblockPlayerComplete::CreateWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete](bool bWasSuccessful, const FString& ErrorMessage)
			{
				if (bWasSuccessful)
				{
					OnGeneratedButtonActionComplete(PlayerController, PlayerData);
				}
			}
		));
	});
	
	// Assign action button to invite player as a friend.
	FTutorialModuleGeneratedWidget* InviteAsFriendButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_invite_as_friend"), AssociateTutorialModule->GeneratedWidgets);
	ensure(InviteAsFriendButtonMetadata);
	InviteAsFriendButtonMetadata->ButtonAction.AddWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete]()
	{
		const FUniqueNetIdRepl InvitedPlayerUserId = PlayerData->UserId;
		ensure(InvitedPlayerUserId.IsValid());

		OnInviteAsFriendButtonClicked(
			PlayerController,
			PlayerData,
			FOnInviteAsFriendPlayerComplete::CreateWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete](bool bWasSuccessful, const FString& ErrorMessage)
			{
				if (bWasSuccessful)
				{
					OnGeneratedButtonActionComplete(PlayerController, PlayerData);
				}
			}
		));
	});
	
	// Assign action button to accept invitation as a friend.
	FTutorialModuleGeneratedWidget* AcceptFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_accept_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(AcceptFriendRequestButtonMetadata);
	AcceptFriendRequestButtonMetadata->ButtonAction.AddWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete]()
	{
		const FUniqueNetIdRepl PlayerUserId = PlayerData->UserId;
		ensure(PlayerUserId.IsValid());

		OnAcceptFriendRequestButtonClicked(
			PlayerController,
			PlayerUserId,
			FOnAcceptFriendRequestComplete::CreateWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete](bool bWasSuccessful, const FString& ErrorMessage)
			{
				if (bWasSuccessful)
				{
					OnGeneratedButtonActionComplete(PlayerController, PlayerData);
				}
			}
		));
	});
	
	// Assign action button to reject invitation as a friend.
	FTutorialModuleGeneratedWidget* RejectFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_reject_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(RejectFriendRequestButtonMetadata);
	RejectFriendRequestButtonMetadata->ButtonAction.AddWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete]()
	{
		const FUniqueNetIdRepl PlayerUserId = PlayerData->UserId;
		ensure(PlayerUserId.IsValid());

		OnRejectFriendRequestButtonClicked(
			PlayerController,
			PlayerUserId,
			FOnRejectFriendRequestComplete::CreateWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete](bool bWasSuccessful, const FString& ErrorMessage)
			{
				// Close the friend details widget if successful.
				if (bWasSuccessful)
				{
					OnGeneratedButtonActionComplete(PlayerController, PlayerData);
				}
			}
		));
	});
	
	// Assign action button to cancel invitation as a friend.
	FTutorialModuleGeneratedWidget* CancelFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_cancel_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(CancelFriendRequestButtonMetadata);
	CancelFriendRequestButtonMetadata->ButtonAction.AddWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete]()
	{
		const FUniqueNetIdRepl PlayerUserId = PlayerData->UserId;
		ensure(PlayerUserId.IsValid());

		OnCancelFriendRequestButtonClicked(
			PlayerController,
			PlayerUserId,
			FOnCancelFriendRequestComplete::CreateWeakLambda(this, [this, PlayerController, PlayerData, OnGeneratedButtonActionComplete](bool bWasSuccessful, const FString& ErrorMessage)
			{
				if (bWasSuccessful)
				{
					OnGeneratedButtonActionComplete(PlayerController, PlayerData);
				}
			}
		));
	});
}

void UManagingFriendsSubsystem::UnbindGeneratedWidgetButtonsAction()
{
	FTutorialModuleGeneratedWidget* UnfriendButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unfriend"), AssociateTutorialModule->GeneratedWidgets);
	ensure(UnfriendButtonMetadata);
	UnfriendButtonMetadata->ButtonAction.RemoveAll(this);
	FTutorialModuleGeneratedWidget* BlockPlayerButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_block_player"), AssociateTutorialModule->GeneratedWidgets);
	ensure(BlockPlayerButtonMetadata);
	BlockPlayerButtonMetadata->ButtonAction.RemoveAll(this);
	FTutorialModuleGeneratedWidget* UnblockPlayerButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unblock"), AssociateTutorialModule->GeneratedWidgets);
	ensure(UnblockPlayerButtonMetadata);
	UnblockPlayerButtonMetadata->ButtonAction.RemoveAll(this);
	FTutorialModuleGeneratedWidget* InviteAsFriendButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_invite_as_friend"), AssociateTutorialModule->GeneratedWidgets);
	ensure(InviteAsFriendButtonMetadata);
	InviteAsFriendButtonMetadata->ButtonAction.RemoveAll(this);
	FTutorialModuleGeneratedWidget* AcceptFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_accept_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(AcceptFriendRequestButtonMetadata);
	AcceptFriendRequestButtonMetadata->ButtonAction.RemoveAll(this);
	FTutorialModuleGeneratedWidget* RejectFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_reject_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(RejectFriendRequestButtonMetadata);
	RejectFriendRequestButtonMetadata->ButtonAction.RemoveAll(this);
	FTutorialModuleGeneratedWidget* CancelFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_cancel_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(CancelFriendRequestButtonMetadata);
	CancelFriendRequestButtonMetadata->ButtonAction.RemoveAll(this);
}

void UManagingFriendsSubsystem::SetGeneratedWidgetButtonsVisibility(UFriendData* PlayerData)
{
	FTutorialModuleGeneratedWidget* UnfriendButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unfriend"), AssociateTutorialModule->GeneratedWidgets);
	ensure(UnfriendButtonMetadata);
	FTutorialModuleGeneratedWidget* BlockPlayerButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_block_player"), AssociateTutorialModule->GeneratedWidgets);
	ensure(BlockPlayerButtonMetadata);
	FTutorialModuleGeneratedWidget* UnblockPlayerButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_unblock"), AssociateTutorialModule->GeneratedWidgets);
	ensure(UnblockPlayerButtonMetadata);
	FTutorialModuleGeneratedWidget* InviteAsFriendButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_invite_as_friend"), AssociateTutorialModule->GeneratedWidgets);
	ensure(InviteAsFriendButtonMetadata);
	FTutorialModuleGeneratedWidget* AcceptFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_accept_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(AcceptFriendRequestButtonMetadata);
	FTutorialModuleGeneratedWidget* RejectFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_reject_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(RejectFriendRequestButtonMetadata);
	FTutorialModuleGeneratedWidget* CancelFriendRequestButtonMetadata = FTutorialModuleGeneratedWidget::GetMetadataById(TEXT("btn_cancel_invitation"), AssociateTutorialModule->GeneratedWidgets);
	ensure(CancelFriendRequestButtonMetadata);
	
	UnfriendButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	BlockPlayerButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	UnblockPlayerButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	InviteAsFriendButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	AcceptFriendRequestButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	RejectFriendRequestButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	CancelFriendRequestButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Collapsed);
	
	switch(PlayerData->Status)
	{
	case EFriendStatus::Accepted:
		UnfriendButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Visible);
		BlockPlayerButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Visible);
		break;
	case EFriendStatus::Blocked:
		UnblockPlayerButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Visible);
		break;
	case EFriendStatus::PendingInbound:
		AcceptFriendRequestButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Visible);
		RejectFriendRequestButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Visible);
		break;
	case EFriendStatus::PendingOutbound:
		CancelFriendRequestButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Visible);
		break;		
	case EFriendStatus::Unknown:
	default:
		InviteAsFriendButtonMetadata->GenerateWidgetRef->SetVisibility(ESlateVisibility::Visible);
		break;
	}
}

void UManagingFriendsSubsystem::UpdateFriendStatus(const APlayerController* PlayerController, UFriendData* PlayerData)
{
	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PlayerController);
	TSharedPtr<FOnlineFriend> Friend = FriendsInterface->GetFriend(LocalUserNum, *PlayerData->UserId, TEXT(""));
	if(Friend != nullptr)
	{
		UFriendData* FriendData = UFriendData::ConvertToFriendData(Friend.ToSharedRef(), this);
		PlayerData->Status = FriendData->Status;
		PlayerData->bCannotBeInvited = FriendData->bCannotBeInvited;
		PlayerData->ReasonCannotBeInvited = FriendData->ReasonCannotBeInvited;
	}
	else
	{
		GetBlockedPlayerList(PlayerController, false, FOnGetBlockedPlayerListComplete::CreateWeakLambda(this, [PlayerData](bool bWasSuccessful, TArray<UFriendData*> BlockedPlayers, const FString& ErrorMessage)
		{
			if(bWasSuccessful)
			{
				if(BlockedPlayers.ContainsByPredicate([PlayerData](const UFriendData* Data)
					{
						return Data->UserId == PlayerData->UserId;
					}))
				{
					PlayerData->Status = EFriendStatus::Blocked;
					PlayerData->bCannotBeInvited = true;
					PlayerData->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Blocked", "Blocked").ToString();
					return;
				}
			}
			PlayerData->Status = EFriendStatus::Unknown;
			PlayerData->bCannotBeInvited = false;
		}));
	}
}

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-GetUniqueNetIdFromPlayerController
FUniqueNetIdPtr UManagingFriendsSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
{
	if (!PC)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-GetLocalUserNumFromPlayerController
int32 UManagingFriendsSubsystem::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
{
	int32 LocalUserNum = 0;

	if (!PC)
	{
		return LocalUserNum;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (LocalPlayer)
	{
		LocalUserNum = LocalPlayer->GetControllerId();
	}

	return LocalUserNum;
}
// @@@SNIPEND

#pragma region Module.12 General Function Definitions

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-OnUnfriendButtonClicked
void UManagingFriendsSubsystem::OnUnfriendButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete)
{
	Unfriend(PC, FriendUserId, OnComplete);
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-OnBlockButtonClicked
void UManagingFriendsSubsystem::OnBlockButtonClicked(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete)
{
	BlockPlayer(PC, BlockedPlayerUserId, OnComplete);
}
// @@@SNIPEND

void UManagingFriendsSubsystem::OnUnblockButtonClicked(const APlayerController* PC,
	const FUniqueNetIdRepl UnblockedPlayerUserId, const FOnUnblockPlayerComplete& OnComplete)
{
	UnblockPlayer(PC, UnblockedPlayerUserId, OnComplete);
}

void UManagingFriendsSubsystem::OnInviteAsFriendButtonClicked(const APlayerController* PC, UFriendData* PlayerData, const FOnInviteAsFriendPlayerComplete& OnComplete)
{
	FriendsSubsystem->SendFriendRequest(
		PC, 
		PlayerData->UserId, 
		FOnSendFriendRequestComplete::CreateWeakLambda(this, [this, PlayerData, OnComplete](bool bWasSuccessful, UFriendData* FriendData, const FString& ErrorMessage) 
		{
			OnComplete.ExecuteIfBound(bWasSuccessful, ErrorMessage);
		}
	));
}

void UManagingFriendsSubsystem::OnAcceptFriendRequestButtonClicked(const APlayerController* PC,
	const FUniqueNetIdRepl PlayerUserId, const FOnAcceptFriendRequestComplete& OnComplete)
{
	FriendsSubsystem->AcceptFriendRequest(PC, PlayerUserId, OnComplete);
}

void UManagingFriendsSubsystem::OnRejectFriendRequestButtonClicked(const APlayerController* PC,
	const FUniqueNetIdRepl PlayerUserId, const FOnRejectFriendRequestComplete& OnComplete)
{
	FriendsSubsystem->RejectFriendRequest(PC, PlayerUserId, OnComplete);
}

void UManagingFriendsSubsystem::OnCancelFriendRequestButtonClicked(const APlayerController* PC,
	const FUniqueNetIdRepl PlayerUserId, const FOnCancelFriendRequestComplete& OnComplete)
{
	FriendsSubsystem->CancelFriendRequest(PC, PlayerUserId, OnComplete);
}

#pragma endregion


#pragma region Module.12 Function Definitions

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-BindOnCachedBlockedPlayersDataUpdated
void UManagingFriendsSubsystem::BindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC, const FOnGetCacheBlockedPlayersDataUpdated& Delegate)
{
	ensure(FriendsInterface);

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

	// Add on blocked players changed delegate.
	OnBlockedPlayersChangeDelegateHandles.Add(LocalUserNum, FriendsInterface->AddOnBlockListChangeDelegate_Handle(LocalUserNum, FOnBlockListChangeDelegate::CreateWeakLambda(this, [Delegate](int32, const FString&) { Delegate.ExecuteIfBound(); })));
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-UnbindOnCachedBlockedPlayersDataUpdated
void UManagingFriendsSubsystem::UnbindOnCachedBlockedPlayersDataUpdated(const APlayerController* PC)
{
	ensure(FriendsInterface);

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);

	// Clear on blocked players changed delegate.
	FDelegateHandle TempHandle = OnBlockedPlayersChangeDelegateHandles[LocalUserNum];
	if (TempHandle.IsValid())
	{
		FriendsInterface->ClearOnFriendsChangeDelegate_Handle(LocalUserNum, TempHandle);
	}
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-GetBlockedPlayerList
void UManagingFriendsSubsystem::GetBlockedPlayerList(const APlayerController* PC, bool bQueryUserInfo, const FOnGetBlockedPlayerListComplete& OnComplete)
{
	if (!ensure(FriendsInterface))
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot cache blocked player list. Friends Interface is not valid."));
		return;
	}

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
	const FUniqueNetIdPtr PlayerNetId = GetUniqueNetIdFromPlayerController(PC);

	// Try to get cached blocked player list first.
	TArray<TSharedRef<FOnlineBlockedPlayer>> CachedBlockedPlayerList;
	if (FriendsInterface->GetBlockedPlayers(PlayerNetId->AsShared().Get(), CachedBlockedPlayerList))
	{
		if(bQueryUserInfo)
		{
			// Then, update the cached blocked players' information by querying their user information.
			TPartyMemberArray BlockedPlayerIds;
			for (const TSharedRef<FOnlineBlockedPlayer>& CachedBlockedPlayer : CachedBlockedPlayerList)
			{
				BlockedPlayerIds.Add(CachedBlockedPlayer.Get().GetUserId());
			}

			// Query blocked players' user information.
			if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UStartupSubsystem>())
			{
				StartupSubsystem->QueryUserInfo(
					LocalUserNum,
					BlockedPlayerIds,
					FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, PlayerNetId, OnComplete, LocalUserNum](
						const FOnlineError& Error,
						const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
					{
						/* Refresh blocked players data with queried blocked players' user information.
						 * Then, return blocked players to the callback. */
						TArray<UFriendData*> BlockedPlayers{};
						TArray<TSharedRef<FOnlineBlockedPlayer>> NewCachedBlockedPlayerList;
						FriendsInterface->GetBlockedPlayers(PlayerNetId->AsShared().Get(), NewCachedBlockedPlayerList);
						for (const TSharedRef<FOnlineBlockedPlayer>& NewCachedBlockedPlayer : NewCachedBlockedPlayerList)
						{
							// Update blocked player's avatar URL based on queried friend's user information.
							FString UserAvatarURL = TEXT("");
							TSharedPtr<FOnlineUser> UserInfo = UserInterface->GetUserInfo(
								LocalUserNum, NewCachedBlockedPlayer.Get().GetUserId().Get());
							UserInfo->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, UserAvatarURL);

							// Add the updated blocked player to the list.
							UFriendData* BlockedPlayer = UFriendData::ConvertToFriendData(NewCachedBlockedPlayer, this);
							BlockedPlayer->AvatarURL = UserAvatarURL;
							BlockedPlayers.Add(BlockedPlayer);
						}

						OnComplete.ExecuteIfBound(true, BlockedPlayers, TEXT(""));
					}));
			}
		}
		else
		{
			TArray<UFriendData*> BlockedPlayers;
			for (const TSharedRef<FOnlineBlockedPlayer>& TempData : CachedBlockedPlayerList)
			{
				BlockedPlayers.Add(UFriendData::ConvertToFriendData(TempData, this));
			}
			
			OnComplete.ExecuteIfBound(true, BlockedPlayers, TEXT(""));
		}
	}
	// If none, request to backend then get the cached the blocked player list.
	else
	{
		FriendsInterface->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(OnQueryBlockedPlayersCompleteDelegateHandle);
		OnQueryBlockedPlayersCompleteDelegateHandle = FriendsInterface->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
			FOnQueryBlockedPlayersCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryBlockedPlayersComplete, OnComplete));
		FriendsInterface->QueryBlockedPlayers(PlayerNetId->AsShared().Get());
	}
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-OnQueryBlockedPlayersComplete
void UManagingFriendsSubsystem::OnQueryBlockedPlayersComplete(const FUniqueNetId& UserId, bool bWasSuccessful, const FString& Error, const FOnGetBlockedPlayerListComplete OnComplete)
{
	 FriendsInterface->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(OnQueryBlockedPlayersCompleteDelegateHandle);

	if (!bWasSuccessful)
	{
		OnComplete.ExecuteIfBound(false, TArray<UFriendData*>(), Error);
		return;
	}

	TArray<TSharedRef<FOnlineBlockedPlayer>> CachedBlockedPlayer{};
	FriendsInterface->GetBlockedPlayers(UserId, CachedBlockedPlayer);

	// Return blocked players to the callback.
	TArray<UFriendData*> BlockedPlayers;
	for (const TSharedRef<FOnlineBlockedPlayer>& TempData : CachedBlockedPlayer)
	{
		BlockedPlayers.Add(UFriendData::ConvertToFriendData(TempData, this));
	}

	OnComplete.ExecuteIfBound(true, BlockedPlayers, TEXT(""));
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-Unfriend
void UManagingFriendsSubsystem::Unfriend(const APlayerController* PC, const FUniqueNetIdRepl FriendUserId, const FOnUnfriendComplete& OnComplete)
{
	if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot unfriend a friend. Friends Interface or Prompt Subsystem is not valid."));
		return;
	}

	PromptSubsystem->ShowLoading(UNFRIEND_FRIEND_MESSAGE);

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
	OnUnfriendCompleteDelegateHandle = FriendsInterface->AddOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, FOnDeleteFriendCompleteDelegate::CreateUObject(this, &ThisClass::OnUnfriendComplete, OnComplete));
	FriendsInterface->DeleteFriend(LocalUserNum, FriendUserId.GetUniqueNetId().ToSharedRef().Get(), TEXT(""));
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-OnUnfriendComplete
void UManagingFriendsSubsystem::OnUnfriendComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr, const FOnUnfriendComplete OnComplete)
{
	PromptSubsystem->HideLoading();

	FriendsInterface->ClearOnDeleteFriendCompleteDelegate_Handle(LocalUserNum, OnUnfriendCompleteDelegateHandle);

	if (bWasSuccessful)
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Success to unfriend a friend."));

		PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_UNFRIEND_FRIEND);
		OnComplete.ExecuteIfBound(true, TEXT(""));
	}
	else
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Failed to unfriend a friend. Error: %s"), *ErrorStr);

		PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
		OnComplete.ExecuteIfBound(false, ErrorStr);
	}
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-BlockPlayer
void UManagingFriendsSubsystem::BlockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnBlockPlayerComplete& OnComplete)
{
	if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot block a player. Friends Interface or Prompt Subsystem is not valid."));
		return;
	}

	PromptSubsystem->ShowLoading(BLOCK_PLAYER_MESSAGE);

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
	OnBlockPlayerCompleteDelegateHandle = FriendsInterface->AddOnBlockedPlayerCompleteDelegate_Handle(LocalUserNum, FOnBlockedPlayerCompleteDelegate::CreateUObject(this, &ThisClass::OnBlockPlayerComplete, OnComplete));
	FriendsInterface->BlockPlayer(LocalUserNum, BlockedPlayerUserId.GetUniqueNetId().ToSharedRef().Get());
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-OnBlockPlayerComplete
void UManagingFriendsSubsystem::OnBlockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnBlockPlayerComplete OnComplete)
{
	PromptSubsystem->HideLoading();

	FriendsInterface->ClearOnBlockedPlayerCompleteDelegate_Handle(LocalUserNum, OnBlockPlayerCompleteDelegateHandle);

	if (bWasSuccessful)
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Success to block a player."));

		PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_BLOCK_PLAYER);
		OnComplete.ExecuteIfBound(true, TEXT(""));
	}
	else
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Failed to block a player. Error: %s"), *ErrorStr);

		PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
		OnComplete.ExecuteIfBound(false, ErrorStr);
	}
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-UnblockPlayer
void UManagingFriendsSubsystem::UnblockPlayer(const APlayerController* PC, const FUniqueNetIdRepl BlockedPlayerUserId, const FOnUnblockPlayerComplete& OnComplete)
{
	if (!ensure(FriendsInterface) || !ensure(PromptSubsystem))
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Cannot unblock a player. Friends Interface or Prompt Subsystem is not valid."));
		return;
	}

	PromptSubsystem->ShowLoading(UNBLOCK_PLAYER_MESSAGE);

	const int32 LocalUserNum = GetLocalUserNumFromPlayerController(PC);
	OnUnblockPlayerCompleteDelegateHandle = FriendsInterface->AddOnUnblockedPlayerCompleteDelegate_Handle(LocalUserNum, FOnBlockedPlayerCompleteDelegate::CreateUObject(this, &ThisClass::OnUnblockPlayerComplete, OnComplete));
	FriendsInterface->UnblockPlayer(LocalUserNum, BlockedPlayerUserId.GetUniqueNetId().ToSharedRef().Get());
}
// @@@SNIPEND

// @@@SNIPSTART ManagingFriendsSubsystem.cpp-OnUnblockPlayerComplete
void UManagingFriendsSubsystem::OnUnblockPlayerComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& BlockedPlayerUserId, const FString& ListName, const FString& ErrorStr, const FOnUnblockPlayerComplete OnComplete)
{
	PromptSubsystem->HideLoading();

	FriendsInterface->ClearOnUnblockedPlayerCompleteDelegate_Handle(LocalUserNum, OnUnblockPlayerCompleteDelegateHandle);

	if (bWasSuccessful)
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Success to unblock a player."));

		PromptSubsystem->ShowMessagePopUp(MESSAGE_PROMPT_TEXT, SUCCESS_UNBLOCK_PLAYER);
		OnComplete.ExecuteIfBound(true, TEXT(""));
	}
	else
	{
		UE_LOG_MANAGING_FRIENDS(Warning, TEXT("Failed to unblock a player. Error: %s"), *ErrorStr);

		PromptSubsystem->ShowMessagePopUp(ERROR_PROMPT_TEXT, FText::FromString(ErrorStr));
		OnComplete.ExecuteIfBound(false, ErrorStr);
	}
}
// @@@SNIPEND

#pragma endregion