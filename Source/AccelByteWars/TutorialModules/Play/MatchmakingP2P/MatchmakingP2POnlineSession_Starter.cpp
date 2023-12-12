// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchmakingP2POnlineSession_Starter.h"

#include "MatchmakingP2PLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/Settings/GameModeDataAssets.h"
#include "Core/UI/InGameMenu/Pause/PauseWidget.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "Core/UI/InGameMenu/GameOver/GameOverWidget.h"
#include "Interfaces/OnlineUserInterface.h"

void UMatchmakingP2POnlineSession_Starter::RegisterOnlineDelegates()
{
	Super::RegisterOnlineDelegates();

	// TODO: Bind your delegates here.
}

void UMatchmakingP2POnlineSession_Starter::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	// TODO: Unbind your delegates here.
}

void UMatchmakingP2POnlineSession_Starter::QueryUserInfo(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& UserIds, const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	// safety
	if (!GetUserInt())
	{
		UE_LOG_MATCHMAKINGP2P(Warning, TEXT("User interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnComplete.ExecuteIfBound(false, {});
		}));
		return;
	}

	TArray<FUserOnlineAccountAccelByte*> UserInfo;
	if (RetrieveUserInfoCache(UserIds, UserInfo))
	{
		UE_LOG_MATCHMAKINGP2P(Log, TEXT("Cache found"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this, UserInfo, OnComplete]()
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

void UMatchmakingP2POnlineSession_Starter::OnQueryUserInfoComplete(
	int32 LocalUserNum,
	bool bSucceeded,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FString& ErrorMessage, const FOnQueryUsersInfoComplete& OnComplete)
{
	UE_LOG_MATCHMAKINGP2P(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

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

		UE_LOG_MATCHMAKINGP2P(Log,
			TEXT("Queried users info: %d, found valid users info: %d"),
			UserIds.Num(), OnlineUsers.Num());

		OnComplete.ExecuteIfBound(true, OnlineUsers);
	}
	else
	{
		OnComplete.ExecuteIfBound(false, {});
	}
}

bool UMatchmakingP2POnlineSession_Starter::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_MATCHMAKINGP2P(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}

#pragma region "Matchmaking with P2P Function Definitions"

// TODO: Add your module function definitions here.

#pragma endregion 
