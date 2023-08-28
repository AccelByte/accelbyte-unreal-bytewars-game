// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsOnlineSessionBase.h"

#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"

#pragma region "Utilities"
FOnlineSessionV2AccelBytePtr UAccelByteWarsOnlineSessionBase::GetABSessionInt()
{
	return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(GetSessionInt());
}

IOnlineIdentityPtr UAccelByteWarsOnlineSessionBase::GetIdentityInt() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return Online::GetIdentityInterface(World);
}

IOnlineUserPtr UAccelByteWarsOnlineSessionBase::GetUserInt() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return Online::GetUserInterface(World);
}

int32 UAccelByteWarsOnlineSessionBase::GetLocalUserNumFromPlayerController(const APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return INDEX_NONE;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return INDEX_NONE;
	}

	return LocalPlayer->GetControllerId();
}

APlayerController* UAccelByteWarsOnlineSessionBase::GetPlayerControllerByUniqueNetId(
	const FUniqueNetIdPtr UniqueNetId) const
{
	APlayerController* MatchedPC = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			continue;
		}
		if (APlayerController* PC = It->Get(); PC)
		{
			const int32 LocalPlayerNum = GetLocalUserNumFromPlayerController(PC);
			if (LocalPlayerNum != INDEX_NONE)
			{
				if (GetIdentityInt()->GetUniquePlayerId(LocalPlayerNum) == UniqueNetId)
				{
					MatchedPC = PC;
					break;
				}
			}
		}
	}
	return MatchedPC;
}

APlayerController* UAccelByteWarsOnlineSessionBase::GetPlayerControllerByLocalUserNum(const int32 LocalUserNum) const
{
	APlayerController* MatchedPC = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			continue;
		}

		if (APlayerController* PC = It->Get(); PC)
		{
			if (LocalUserNum == GetLocalUserNumFromPlayerController(PC))
			{
				MatchedPC = PC;
				break;
			}
		}
	}
	return MatchedPC;
}

FUniqueNetIdPtr UAccelByteWarsOnlineSessionBase::GetLocalPlayerUniqueNetId(
	const APlayerController* PlayerController) const
{
	if (!ensure(PlayerController)) 
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!ensure(LocalPlayer))
	{
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

bool UAccelByteWarsOnlineSessionBase::CompareAccelByteUniqueId(
	const FUniqueNetIdRepl& FirstUniqueNetId,
	const FUniqueNetIdRepl& SecondUniqueNetId) const
{
	if (!FirstUniqueNetId.IsValid() || !SecondUniqueNetId.IsValid())
	{
		return false;
	}

	// compare directly first
	if (FirstUniqueNetId == SecondUniqueNetId)
	{
		return true;
	}

	// if false, attempt to compare AB User Id first
	const FUniqueNetIdAccelByteUserPtr FirstAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*FirstUniqueNetId);
	const FUniqueNetIdAccelByteUserPtr SecondAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*SecondUniqueNetId);

	if (!FirstAbUniqueNetId.IsValid() || !SecondAbUniqueNetId.IsValid())
	{
		return false;
	}

	const FString FirstAbUserId = FirstAbUniqueNetId->GetAccelByteId();
	const FString SecondAbUserId = SecondAbUniqueNetId->GetAccelByteId();

	return FirstAbUserId.Equals(SecondAbUserId);
}

bool UAccelByteWarsOnlineSessionBase::CompareAccelByteUniqueId(
	const FUniqueNetIdRepl& FirstUniqueNetId,
	const FString& SecondAbUserId) const
{
	if (!FirstUniqueNetId.IsValid())
	{
		return false;
	}

	const FUniqueNetIdAccelByteUserPtr FirstAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*FirstUniqueNetId);
	if (!FirstAbUniqueNetId.IsValid())
	{
		return false;
	}
	const FString FirstAbUserId = FirstAbUniqueNetId->GetAccelByteId();

	return FirstAbUserId.Equals(SecondAbUserId);
}
#pragma endregion 

#pragma region "Game Session Essentials | Query caching workaround"
bool UAccelByteWarsOnlineSessionBase::RetrieveUserInfoCache(
	const TArray<FUniqueNetIdRef>& UserIds,
	TArray<FUserOnlineAccountAccelByte*>& OutUserInfo)
{
	// safety
	if (!GetUserInt())
	{
		return false;
	}

	bool bMissingCache = false;
	for (const FUniqueNetIdRef& UserId : UserIds)
	{
		FUserOnlineAccountAccelByte* UserInfo =
			CachedUsersInfo.FindByPredicate([UserId, this](const FUserOnlineAccountAccelByte& UserOnline)
			{
				return CompareAccelByteUniqueId(UserOnline.GetUserId(), UserId);
			});

		if (UserInfo != nullptr)
		{
			OutUserInfo.Add(UserInfo);
			continue;
		}

		bMissingCache = true;
	}

	return !bMissingCache;
}

bool UAccelByteWarsOnlineSessionBase::DSRetrieveUserInfoCache(
	const TArray<FUniqueNetIdRef>& UserIds,
	TArray<const FBaseUserInfo*> OutUserInfo)
{
	bool bMissingCache = false;
	for (const FUniqueNetIdRef& UserId : UserIds)
	{
		const FUniqueNetIdAccelByteUserPtr AbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(UserId);
		if (AbUniqueNetId.IsValid())
		{
			const FBaseUserInfo* UserInfo =
				DSCachedUsersInfo.FindByPredicate([AbUniqueNetId](const FBaseUserInfo& UserInfo)
				{
					return UserInfo.UserId.Compare(AbUniqueNetId->GetAccelByteId());
				});
			if (UserInfo != nullptr)
			{
				OutUserInfo.Add(UserInfo);
				continue;
			}
		}

		bMissingCache = true;
		break;
	}

	return !bMissingCache;
}

void UAccelByteWarsOnlineSessionBase::CacheUserInfo(const int32 LocalUserNum, const TArray<FUniqueNetIdRef>& UserIds)
{
	for (const FUniqueNetIdRef& UserId : UserIds)
	{
		TSharedPtr<FOnlineUser> OnlineUserPtr = GetUserInt()->GetUserInfo(LocalUserNum, UserId.Get());
		TSharedPtr<FUserOnlineAccountAccelByte> AbOnlineUserPtr = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(OnlineUserPtr);
		const FUserOnlineAccountAccelByte* OnlineUser = AbOnlineUserPtr.Get();

		if (OnlineUserPtr.IsValid() && OnlineUser)
		{
			// store to own cache as a workaround to OSS cache occasionally missing its data
			CachedUsersInfo.RemoveAll([OnlineUser, this](const FUserOnlineAccountAccelByte& SearchedUser)
			{
				return CompareAccelByteUniqueId(SearchedUser.GetUserId(), OnlineUser->GetUserId());
			});
			CachedUsersInfo.Add(*OnlineUser);
		}
	}
}

void UAccelByteWarsOnlineSessionBase::CacheUserInfo(const FListBulkUserInfo& UserInfoList)
{
	for (const FBaseUserInfo& User : UserInfoList.Data)
	{
		// store to own cache as a workaround to OSS cache occasionally missing its data
		DSCachedUsersInfo.RemoveAll([User](const FBaseUserInfo& SearchedUser)
		{
			return SearchedUser.UserId.Compare(User.UserId);
		});
		DSCachedUsersInfo.Add(User);
	}
}
#pragma endregion

#pragma region "Match Session Essentials | Utilities"
TArray<FMatchSessionEssentialInfo> UAccelByteWarsOnlineSessionBase::SimplifySessionSearchResult(
	const TArray<FOnlineSessionSearchResult>& SearchResults,
	const TArray<FUserOnlineAccountAccelByte*> UsersInfo,
	const TMap<TPair<EGameModeNetworkType, EGameModeType>,
	FString>& PredefinedSessionTemplateNames)
{
	TArray<FMatchSessionEssentialInfo> SessionEssentialInfos;

	for (const FOnlineSessionSearchResult& SearchResult : SearchResults)
	{
		const FOnlineSession& Session = SearchResult.Session;
		const FOnlineSessionSettings& SessionSettings = Session.SessionSettings;

		// failsafe: make sure this is a game session
		if (GetABSessionInt()->GetSessionTypeFromSettings(SessionSettings) != EAccelByteV2SessionType::GameSession)
		{
			continue;
		}

		FMatchSessionEssentialInfo SessionEssentialInfo{SearchResult};

		// search the owner's info
		FString OwnerUsername = "";
		FString OwnerAvatarUrl = "";
		for (const FUserOnlineAccountAccelByte* User : UsersInfo)
		{
			if (!User || !Session.OwningUserId.IsValid())
			{
				continue;
			}

			if (CompareAccelByteUniqueId(FUniqueNetIdRepl(Session.OwningUserId), FUniqueNetIdRepl(User->GetUserId())))
			{
				// Session.OwningUserName is not retrieved during QuerySession
				OwnerUsername = User->GetDisplayName();
				User->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, OwnerAvatarUrl);
				break;
			}
		}

		// store the info
		SessionEssentialInfo.OwnerAvatarUrl = OwnerAvatarUrl;
		SessionEssentialInfo.OwnerName = OwnerUsername.IsEmpty() ? "Player" : OwnerUsername;

		// network type
		FString ServerType;
		SessionSettings.Get(SETTING_SESSION_SERVER_TYPE, ServerType); // "DS" / "P2P"
		SessionEssentialInfo.NetworkType =
			ServerType.ToUpper().Equals("P2P") ? EGameModeNetworkType::P2P : EGameModeNetworkType::DS;

		// game mode type
		FString SessionTemplateName;
		SessionSettings.Get(SETTING_SESSION_TEMPLATE_NAME, SessionTemplateName);
		for (const TTuple<TPair<EGameModeNetworkType, EGameModeType>, FString>& TemplateName : PredefinedSessionTemplateNames)
		{
			if (TemplateName.Value.Equals(SessionTemplateName))
			{
				SessionEssentialInfo.GameModeType = TemplateName.Key.Value;
				break;
			}
		}

		// Max player num
		SessionEssentialInfo.MaxPlayerCount = SessionSettings.NumPublicConnections;

		// Registered player num
		TSharedPtr<FOnlineSessionInfo> SessionInfo = Session.SessionInfo;
		if (!SessionInfo.IsValid())
		{
			continue;
		}

		TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
		if (!AbSessionInfo.IsValid())
		{
			continue;
		}

		TSharedPtr<FAccelByteModelsV2BaseSession> AbBaseSessionInfo = AbSessionInfo->GetBackendSessionData();
		if (!AbBaseSessionInfo.IsValid())
		{
			continue;
		}

		int32 ActiveMemberCount = 0;
		TArray<FAccelByteModelsV2SessionUser> AbMembers = AbBaseSessionInfo->Members;
		for (const FAccelByteModelsV2SessionUser& Member : AbMembers)
		{
			if (Member.StatusV2 == EAccelByteV2SessionMemberStatus::JOINED)
			{
				ActiveMemberCount++;
			}
		}
		SessionEssentialInfo.RegisteredPlayerCount = ActiveMemberCount;

		SessionEssentialInfos.Add(SessionEssentialInfo);
	}

	return SessionEssentialInfos;
}
#pragma endregion
