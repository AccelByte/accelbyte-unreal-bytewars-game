// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchSessionDSOnlineSession_Starter.h"

#include "MatchSessionDSLog.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"

void UMatchSessionDSOnlineSession_Starter::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	const TDelegate<void(APlayerController*)> LeaveSessionDelegate = TDelegate<void(APlayerController*)>::CreateWeakLambda(
		this, [this](APlayerController*)
		{
			LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		});
	UPauseWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);
	UMatchLobbyWidget::OnQuitLobbyDelegate.Add(LeaveSessionDelegate);
	UGameOverWidget::OnQuitGameDelegate.Add(LeaveSessionDelegate);

	// TODO: Add your delegates setup here
}

void UMatchSessionDSOnlineSession_Starter::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);
	UGameOverWidget::OnQuitGameDelegate.RemoveAll(this);

	// TODO: Add your delegates cleanup here
}

void UMatchSessionDSOnlineSession_Starter::QueryUserInfo(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	// safety
	if (!GetUserInt())
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("User interface null"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnComplete.ExecuteIfBound(false, {});
		}));
		return;
	}

	TArray<FUserOnlineAccountAccelByte*> UserInfo;
	if (RetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHSESSIONDS(Log, TEXT("Cache found"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	// Some data does not exist in cache, query everything
	else
	{
		// Bind delegate
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

void UMatchSessionDSOnlineSession_Starter::DSQueryUserInfo(
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	const TArray<const FBaseUserInfo*> UserInfo;
	if (DSRetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHSESSIONDS(Log, TEXT("Cache found"))
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete, UserInfo]()
		{
			OnComplete.ExecuteIfBound(true, UserInfo);
		}));
	}
	else
	{
		// gather user ids
		TArray<FString> AbUserIds;
		for (const FUniqueNetIdRef& UserId : UserIds)
		{
			const FUniqueNetIdAccelByteUserPtr AbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*UserId);
			const FString AbUserId = AbUniqueNetId->GetAccelByteId();
			if (!AbUserId.IsEmpty())
			{
				AbUserIds.Add(AbUserId);
			}
		}

		AccelByte::FRegistry::User.BulkGetUserInfo(
			AbUserIds,
			THandler<FListBulkUserInfo>::CreateWeakLambda(this, [OnComplete, this](FListBulkUserInfo UserInfo)
			{
				OnDSQueryUserInfoComplete(UserInfo, OnComplete);
			}),
			FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32, const FString&)
			{
				ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
				{
					OnDSQueryUserInfoComplete(FListBulkUserInfo(), OnComplete);
				}));
			})
		);
	}
}

void UMatchSessionDSOnlineSession_Starter::OnQueryUserInfoComplete(
	int32 LocalUserNum,
	bool bSucceeded,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FString& ErrorMessage,
	const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"))

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

		UE_LOG_MATCHSESSIONDS(Log,
			TEXT("Queried users info: %d, found valid users info: %d"),
			UserIds.Num(), OnlineUsers.Num());

		OnComplete.ExecuteIfBound(true, OnlineUsers);
	}
	else
	{
		OnComplete.ExecuteIfBound(false, {});
	}
}

void UMatchSessionDSOnlineSession_Starter::OnDSQueryUserInfoComplete(
	const FListBulkUserInfo& UserInfoList,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	// reset delegate handle
	OnDSQueryUserInfoCompleteDelegateHandle.Reset();

	if (UserInfoList.Data.IsEmpty())
	{
		OnComplete.ExecuteIfBound(false, {});
	}

	CacheUserInfo(UserInfoList);

	TArray<const FBaseUserInfo*> OnlineUsers;
	for (const FBaseUserInfo& User : UserInfoList.Data)
	{
		OnlineUsers.Add(&User);
	}
	OnComplete.ExecuteIfBound(true, OnlineUsers);
}

void UMatchSessionDSOnlineSession_Starter::OnQueryUserInfoForFindSessionComplete(
	const bool bSucceeded,
	const TArray<FUserOnlineAccountAccelByte*>& UsersInfo)
{
	UE_LOG_MATCHSESSIONDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"))

	if (bSucceeded)
	{
		const TArray<FMatchSessionEssentialInfo> MatchSessionSearchResult = SimplifySessionSearchResult(
			SessionSearch->SearchResults,
			UsersInfo,
			MatchSessionTemplateNameMap);

		OnFindSessionsCompleteDelegates.Broadcast(MatchSessionSearchResult, true);
	}
	else
	{
		OnFindSessionsCompleteDelegates.Broadcast({}, false);
	}
}

#pragma region "Matchmaking with DS implementations"
// TODO: Add your module implementations here.
#pragma endregion 
