// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchSessionDSOnlineSession_Starter.h"

#include "MatchSessionDSLog.h"
#include "OnlineSessionSettingsAccelByte.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem_Starter.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

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

void UMatchSessionDSOnlineSession_Starter::DSQueryUserInfo(
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("Called"))

	AccelByte::FServerApiClientPtr ServerApiClient = UTutorialModuleOnlineUtility::GetServerApiClient(this);
	if (!ServerApiClient)
	{
		UE_LOG_MATCHSESSIONDS(Log, TEXT("Cannot query user info. Server API Client is invalid."));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnDSQueryUserInfoComplete({}, OnComplete);
		}));
		return;
	}

	const TArray<const FUserDataResponse*> UserInfo;
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
		// Gather user IDs.
		FListUserDataRequest Request;
		for (const FUniqueNetIdRef& UserId : UserIds)
		{
			const FUniqueNetIdAccelByteUserPtr AbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*UserId);
			const FString AbUserId = AbUniqueNetId->GetAccelByteId();
			if (!AbUserId.IsEmpty())
			{
				Request.UserIds.Add(AbUserId);
			}
		}

		ServerApiClient->ServerUser.ListUserByUserId(
			Request,
			THandler<FListUserDataResponse>::CreateWeakLambda(this, [OnComplete, this](FListUserDataResponse UserInfo)
			{
				OnDSQueryUserInfoComplete(UserInfo, OnComplete);
			}),
			FErrorHandler::CreateWeakLambda(this, [this, OnComplete](int32, const FString&)
			{
				ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
				{
					OnDSQueryUserInfoComplete({}, OnComplete);
				}));
			})
		);
	}
}

void UMatchSessionDSOnlineSession_Starter::OnDSQueryUserInfoComplete(
	const FListUserDataResponse& UserInfoList,
	const FOnDSQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("Called"))

	// Reset delegate handle
	OnDSQueryUserInfoCompleteDelegateHandle.Reset();

	if (UserInfoList.Data.IsEmpty())
	{
		OnComplete.ExecuteIfBound(false, {});
	}

	CacheUserInfo(UserInfoList);

	TArray<const FUserDataResponse*> OnlineUsers;
	for (const FUserDataResponse& User : UserInfoList.Data)
	{
		OnlineUsers.Add(&User);
	}
	OnComplete.ExecuteIfBound(true, OnlineUsers);
}

void UMatchSessionDSOnlineSession_Starter::OnQueryUserInfoForFindSessionComplete(
	const FOnlineError& Error,
	const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
{
	UE_LOG_MATCHSESSIONDS(Log, TEXT("succeeded: %s"), *FString(Error.bSucceeded ? TEXT("TRUE") : TEXT("FALSE")))

	if (Error.bSucceeded)
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
