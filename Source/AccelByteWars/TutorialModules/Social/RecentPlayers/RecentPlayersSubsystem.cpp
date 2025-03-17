// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RecentPlayersSubsystem.h"
#include "RecentPlayersLog.h"

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"

#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "Social/FriendsEssentials/UI/FriendDetailsWidget.h"
#include "Social/ManagingFriends/ManagingFriendsSubsystem.h"

#include "Core/System/AccelByteWarsGameInstance.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

// @@@SNIPSTART RecentPlayersSubsystem.cpp-Initialize
// @@@MULTISNIP BindOnSessionDestroyedDelegate {"selectedLines": ["1-2", "25-26"]}
void URecentPlayersSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return;
	}
	
	FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
	if (!ensure(FriendsInterface.IsValid()))
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Friends Interface is not valid."));
		return;
	}

	SessionInterface = Subsystem->GetSessionInterface();
	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Session Interface is not valid."));
		return;
	}
	
	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnSessionDestroyed));
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-Deinitialize
// @@@MULTISNIP UnbindOnSessionDestroyedDelegate {"selectedLines": ["1-2", "4-5"]}
void URecentPlayersSubsystem::Deinitialize()
{
	Super::Deinitialize();
	SessionInterface->ClearOnDestroySessionCompleteDelegates(this);
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-OnSessionDestroyed
void URecentPlayersSubsystem::OnSessionDestroyed(FName SessionName, bool bWasSuccessful)
{
	// refresh recent player if a game session is destroyed
	if(bWasSuccessful && SessionName.IsEqual(NAME_GameSession))
	{
		const UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
		ensure(GameInstance);

		QueryRecentPlayers(GameInstance->GetFirstLocalPlayerController());
	}
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-GetRecentPlayers
void URecentPlayersSubsystem::GetRecentPlayers(const APlayerController* PlayerController,
	const FOnGetRecentPlayersComplete& OnComplete)
{
	const FUniqueNetIdPtr LocalPlayerId = GetUniqueNetIdFromPlayerController(PlayerController);
	if (!ensure(LocalPlayerId.IsValid()))
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Cannot get recent player. LocalPlayer NetId is not valid."));
		return;
	}

	TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
	bool bSuccess = FriendsInterface->GetRecentPlayers(LocalPlayerId.ToSharedRef().Get(), TEXT(""), RecentPlayers);

	if(bSuccess)
	{
		UE_LOG_RECENTPLAYERS(Log, TEXT("Success to get recent player list."));

		RecentPlayersData.Empty();
		for(const TSharedRef<FOnlineRecentPlayer>& Player: RecentPlayers)
		{
			RecentPlayersData.Add(UFriendData::ConvertToFriendData(Player));
		}

		UpdatePlayersInviteStatus(PlayerController, OnComplete, RecentPlayersData);
	}
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-QueryRecentPlayers
void URecentPlayersSubsystem::QueryRecentPlayers(const APlayerController* PlayerController)
{
	const FUniqueNetIdPtr LocalPlayerId = GetUniqueNetIdFromPlayerController(PlayerController);
	if (!ensure(LocalPlayerId.IsValid()))
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Cannot query recent player. LocalPlayer NetId is not valid."));
		return;
	}
	
	FriendsInterface->QueryRecentPlayers(LocalPlayerId.ToSharedRef().Get(), TEXT(""));
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-GetUniqueNetIdFromPlayerController
FUniqueNetIdPtr URecentPlayersSubsystem::GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-BindRecentPlayerDelegate
FDelegateHandle URecentPlayersSubsystem::BindRecentPlayerDelegate(FOnQueryRecentPlayersCompleteDelegate& Delegate)
{
	return FriendsInterface->AddOnQueryRecentPlayersCompleteDelegate_Handle(Delegate);
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-UnBindRecentPlayerDelegate
void URecentPlayersSubsystem::UnBindRecentPlayerDelegate(FDelegateHandle& Delegate)
{
	FriendsInterface->ClearOnQueryRecentPlayersCompleteDelegate_Handle(Delegate);
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-GetGameSessionPlayerList
void URecentPlayersSubsystem::GetGameSessionPlayerList(const APlayerController* PlayerController, const FOnGetGameSessionPlayerListComplete& OnComplete)
{
	const FNamedOnlineSession* GameSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if(!GameSession)
	{
		return;
	}
	
	const TSharedPtr<FOnlineSessionInfo> SessionInfo = GameSession->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		return;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		return;
	}

	const TSharedPtr<FAccelByteModelsV2BaseSession> AbBaseSessionInfo = AbSessionInfo->GetBackendSessionData();
	if (!AbBaseSessionInfo.IsValid())
	{
		return;
	}

	const FUniqueNetIdAccelByteUserPtr TargetUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(GetUniqueNetIdFromPlayerController(PlayerController));

	if(!TargetUserABId.IsValid())
	{
		UE_LOG_RECENTPLAYERS(Warning, TEXT("Local userid invalid"));
		return;
	}

	GameSessionPlayersData.Empty();
	
	TArray<FAccelByteModelsV2SessionUser> AbMembers = AbBaseSessionInfo->Members;
	TArray<FUniqueNetIdRef> UniqueNetIds;
	for (const FAccelByteModelsV2SessionUser& AbMember : AbMembers)
	{
		// skip local player
		if(TargetUserABId->GetAccelByteId().Equals(AbMember.ID))
		{
			continue;
		}
		
		FAccelByteUniqueIdComposite CompositeId;
		CompositeId.Id = AbMember.ID;

		FUniqueNetIdAccelByteUserRef AccelByteUser = FUniqueNetIdAccelByteUser::Create(CompositeId);
		UniqueNetIds.Add(AccelByteUser);

		const FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
		if (!ensure(Subsystem))
		{
			UE_LOG_RECENTPLAYERS(Warning, TEXT("The online subsystem is invalid."));
			return;
		}
		
		const TSharedPtr<const FAccelByteUserInfo> User = Subsystem->GetUserCache()->GetUser(AccelByteUser.Get());
		if(User.IsValid())
		{
			UFriendData* PlayerData = UFriendData::ConvertToFriendData(User.ToSharedRef());
			GameSessionPlayersData.Add(PlayerData);
		}
		else
		{
			UE_LOG_RECENTPLAYERS(Warning, TEXT("User is invalid"));
		}
	}

	UE_LOG_RECENTPLAYERS(Log, TEXT("Success to get game session player list."));

	UpdatePlayersInviteStatus(PlayerController, OnComplete, GameSessionPlayersData);
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-GetGameSessionPlayerStatus
FString URecentPlayersSubsystem::GetGameSessionPlayerStatus(UFriendData* Player)
{
	FString StatusAsString = TEXT("");
	const FNamedOnlineSession* GameSession = SessionInterface->GetNamedSession(NAME_GameSession);
	const TSharedPtr<FOnlineSessionInfo> SessionInfo = GameSession->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		return StatusAsString;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		return StatusAsString;
	}

	const TSharedPtr<FAccelByteModelsV2BaseSession> AbBaseSessionInfo = AbSessionInfo->GetBackendSessionData();
	if (!AbBaseSessionInfo.IsValid())
	{
		return StatusAsString;
	}
	
	TArray<FAccelByteModelsV2SessionUser> AbMembers = AbBaseSessionInfo->Members;
	const FUniqueNetIdAccelByteUserPtr TargetUserABId = StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(Player->UserId);
	FAccelByteModelsV2SessionUser* OnlineUser = AbMembers.FindByPredicate([TargetUserABId](FAccelByteModelsV2SessionUser User)
	{
		return User.ID.Equals(TargetUserABId->GetAccelByteId());
	});
	if(OnlineUser != nullptr)
	{
		UEnum::GetValueAsString<EAccelByteV2SessionMemberStatus>(OnlineUser->StatusV2, StatusAsString);
		StatusAsString = StatusAsString.RightChop(StatusAsString.Find(TEXT("::")) + 2);
	}
	
	return StatusAsString;
}
// @@@SNIPEND

// @@@SNIPSTART RecentPlayersSubsystem.cpp-UpdatePlayersInviteStatus
void URecentPlayersSubsystem::UpdatePlayersInviteStatus(const APlayerController* PlayerController, const FOnGetGameSessionPlayerListComplete& OnComplete, TArray<UFriendData*>& PlayersData)
{
	UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	UFriendsSubsystem* FriendsSubsystem = GameInstance->GetSubsystem<UFriendsSubsystem>();
	ensure(FriendsSubsystem);

	FriendsSubsystem->GetFriendsInviteStatus(PlayerController, PlayersData, FOnGetPlayersInviteStatusComplete::CreateWeakLambda(this, [this, PlayerController, OnComplete, &PlayersData, GameInstance](bool bWasSuccessful, const FString& ErrorMessage)
	{
		if(bWasSuccessful)
		{
			//check blocked player list, as blocked player returned from Friends subsystem has friend status as EFriendStatus::Unknown
			UManagingFriendsSubsystem* ManagingFriendsSubsystem = GameInstance->GetSubsystem<UManagingFriendsSubsystem>();
			ensure(ManagingFriendsSubsystem);
			ManagingFriendsSubsystem->GetBlockedPlayerList(
				PlayerController,
				false,
				FOnGetBlockedPlayerListComplete::CreateWeakLambda(this, [this, OnComplete, &PlayersData](bool bWasSuccessful, TArray<UFriendData*> BlockedPlayers, const FString& ErrorMessage)
				{
					if(bWasSuccessful)
					{
						for(UFriendData* PlayerData: PlayersData)
						{
							if(BlockedPlayers.ContainsByPredicate([PlayerData](const UFriendData* Data)
								{
									return Data->UserId == PlayerData->UserId;
								}))
							{
								PlayerData->Status = EFriendStatus::Blocked;
								PlayerData->bCannotBeInvited = true;
								PlayerData->ReasonCannotBeInvited = NSLOCTEXT("AccelByteWars", "Blocked", "Blocked").ToString();
							}
						}
					}
						
					OnComplete.ExecuteIfBound(bWasSuccessful, PlayersData);
				}));
		}
		else
		{
			OnComplete.ExecuteIfBound(false, GameSessionPlayersData);
			UE_LOG_RECENTPLAYERS(Warning, TEXT("Failed get invite status. Error message: %s"), *ErrorMessage);
		}
	}));
}
// @@@SNIPEND
