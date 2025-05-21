// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsOnlineSessionBase.h"

#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"

#pragma region "Utilities"
// @@@SNIPSTART AccelByteWarsOnlineSessionBase.cpp-GetABSessionInt
FOnlineSessionV2AccelBytePtr UAccelByteWarsOnlineSessionBase::GetABSessionInt()
{
	return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(GetSessionInt());
}
// @@@SNIPEND

IOnlineIdentityPtr UAccelByteWarsOnlineSessionBase::GetIdentityInt() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return Online::GetIdentityInterface(World);
}

FOnlineIdentityAccelBytePtr UAccelByteWarsOnlineSessionBase::GetABIdentityInt() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Online::GetIdentityInterface(World));
}

// @@@SNIPSTART AccelByteWarsOnlineSessionBase.cpp-GetUserInt
IOnlineUserPtr UAccelByteWarsOnlineSessionBase::GetUserInt() const
{
	const UWorld* World = GetWorld();
	if (!ensure(World))
	{
		return nullptr;
	}

	return Online::GetUserInterface(World);
}
// @@@SNIPEND

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
	if (!FirstUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE) ||
		!SecondUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE))
	{
		return false;
	}
	
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
	if (!FirstUniqueNetId.IsValid() || !FirstUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE))
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
bool UAccelByteWarsOnlineSessionBase::DSRetrieveUserInfoCache(
	const TArray<FUniqueNetIdRef>& UserIds,
	TArray<const FUserDataResponse*> OutUserInfo)
{
	bool bMissingCache = false;
	for (const FUniqueNetIdRef& UserId : UserIds)
	{
		const FUniqueNetIdAccelByteUserPtr AbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(UserId);
		if (AbUniqueNetId.IsValid())
		{
			const FUserDataResponse* UserInfo = 
				DSCachedUsersInfo.FindByPredicate([AbUniqueNetId](const FUserDataResponse& UserInfo)
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

void UAccelByteWarsOnlineSessionBase::CacheUserInfo(const FListUserDataResponse& UserInfoList)
{
	for (const FUserDataResponse& User : UserInfoList.Data)
	{
		// Store to own cache as a workaround to OSS cache occasionally missing its data
		DSCachedUsersInfo.RemoveAll([User](const FUserDataResponse& SearchedUser)
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
	const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo,
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
		for (const TSharedPtr<FUserOnlineAccountAccelByte>& User : UsersInfo)
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
			// Assuming the TemplateName.Value contains game mode of type "elimination" or "teamdeathmatch".
			if (TemplateName.Value.Contains(SessionTemplateName))
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
