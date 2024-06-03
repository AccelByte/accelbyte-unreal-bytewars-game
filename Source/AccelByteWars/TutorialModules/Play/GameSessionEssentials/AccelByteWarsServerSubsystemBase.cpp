// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "AccelByteWarsServerSubsystemBase.h"

#include "GameSessionEssentialsLog.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemAccelByteTypes.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UAccelByteWarsServerSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG_GAMESESSION(Verbose, TEXT("called"))

	AAccelByteWarsGameMode::OnPlayerPostLoginDelegates.AddUObject(this, &ThisClass::AuthenticatePlayer);
	AAccelByteWarsGameSession::OnRegisterServerDelegates.AddUObject(this, &ThisClass::RegisterServer);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.AddUObject(this, &ThisClass::UnregisterServer);
	AAccelByteWarsGameMode::OnInitializeListenServerDelegates.AddUObject(this, &ThisClass::OnServerSessionReceived);
	AAccelByteWarsInGameGameMode::OnGameEndsDelegate.AddUObject(this, &ThisClass::CloseGameSession);

	UOnlineSession* OnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(OnlineSession))
	{
		return;
	}
	GameSessionOnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(OnlineSession);
	if (ensure(GameSessionOnlineSession))
	{
		return;
	}

	GameSessionOnlineSession->GetOnLeaveSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnLeaveSessionComplete);
}

void UAccelByteWarsServerSubsystemBase::Deinitialize()
{
	Super::Deinitialize();

	AAccelByteWarsGameMode::OnPlayerPostLoginDelegates.RemoveAll(this);
	AAccelByteWarsGameSession::OnRegisterServerDelegates.RemoveAll(this);
	AAccelByteWarsGameSession::OnUnregisterServerDelegates.RemoveAll(this);

	AAccelByteWarsGameMode::OnInitializeListenServerDelegates.RemoveAll(this);
	AAccelByteWarsInGameGameMode::OnGameEndsDelegate.RemoveAll(this);

	GameSessionOnlineSession->GetOnLeaveSessionCompleteDelegates()->RemoveAll(this);
}

void UAccelByteWarsServerSubsystemBase::OnServerSessionReceived(FName SessionName)
{
	bHasReceivedSession = true;
	if (!IsRunningDedicatedServer())
	{
		UpdateUserCache();
	}
}

IOnlineSessionPtr UAccelByteWarsServerSubsystemBase::GetSessionInt() const
{
	IOnlineSessionPtr SessionInt = Online::GetSessionInterface();
	ensure(SessionInt);

	return SessionInt;
}

FOnlineSessionV2AccelBytePtr UAccelByteWarsServerSubsystemBase::GetABSessionInt() const
{
	FOnlineSessionV2AccelBytePtr ABSessionInt = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(GetSessionInt());
	ensure(ABSessionInt);

	return ABSessionInt;
}

void UAccelByteWarsServerSubsystemBase::ExecuteNextTick(const FTimerDelegate & Delegate) const
{
	GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
}

void UAccelByteWarsServerSubsystemBase::OnLeaveSessionComplete(FName SessionName, bool bSucceeded)
{
	if (bSucceeded)
	{
		bHasReceivedSession = false;
	}
}

void UAccelByteWarsServerSubsystemBase::UpdateUserCache()
{
	const FNamedOnlineSession* NamedOnlineSession =
			GameSessionOnlineSession->GetSession(
				GameSessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	if (!NamedOnlineSession)
	{
		return;
	}
		
	const TSharedPtr<FOnlineSessionInfo> SessionInfo = NamedOnlineSession->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		return;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		return;
	}
	TArray<FAccelByteModelsV2GameSessionTeam> Teams = AbSessionInfo->GetTeamAssignments();
	for (int i = 0; i < Teams.Num(); ++i)
	{
		for (const FString& UserId : Teams[i].UserIDs)
		{
			for (TTuple<FUniqueNetIdRepl, TTuple<FUserOnlineAccountAccelByte, int>>& UserInfo : CachedUsersInfo)
			{
				if (GameSessionOnlineSession->CompareAccelByteUniqueId(UserInfo.Key, UserId))
				{
					UserInfo.Value.Value = i;
				}
			}
		}
	}	
}

void UAccelByteWarsServerSubsystemBase::CloseGameSession()
{
	UE_LOG_GAMESESSION(Verbose, TEXT("called"));

	if (!GameSessionOnlineSession)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Failed to close game session joinability. Online Session is null"));
		return;
	}

	const FName GameSession = GameSessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession);
	GameSessionOnlineSession->UpdateSessionJoinability(GameSession, EAccelByteV2SessionJoinability::CLOSED);
}

#pragma region "Authenticating player"
void UAccelByteWarsServerSubsystemBase::AuthenticatePlayer(APlayerController* PlayerController)
{
	UE_LOG_GAMESESSION(Verbose, TEXT("called"))

	const ENetMode NetMode = GetWorld()->GetNetMode();
	if (NetMode == ENetMode::NM_Standalone || NetMode == ENetMode::NM_Client)
	{
		return;
	}

	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!PlayerState)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Player State is invalid"));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]()
		{
			OnAuthenticatePlayerComplete(PlayerController, false);
		}));
		return;
	}

	AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AbPlayerState)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Player State is not derived from AAccelByteWarsPlayerState"));
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]()
		{
			OnAuthenticatePlayerComplete(PlayerController, false);
		}));
		return;
	}

	const FUniqueNetIdRepl& PlayerUniqueNetId = PlayerState->GetUniqueId();

	// check cache
	if (IsRunningDedicatedServer())
	{
		for (const TTuple<FString, TTuple<FBaseUserInfo, int>>& UserInfo : DSCachedUsersInfo)
		{
			if (GameSessionOnlineSession->CompareAccelByteUniqueId(PlayerUniqueNetId, UserInfo.Key) &&
				UserInfo.Value.Value != INDEX_NONE)
			{
				UE_LOG_GAMESESSION(Log, TEXT("Found DS cache"));

				// cache found, trigger immediately
				AbPlayerState->SetPlayerName(UserInfo.Value.Key.DisplayName);
				AbPlayerState->TeamId = UserInfo.Value.Value;
				AbPlayerState->AvatarURL = UserInfo.Value.Key.AvatarUrl;

				ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]()
				{
					OnAuthenticatePlayerComplete(PlayerController, true);
				}));
				return;
			}
		}
	}
	else
	{
		for (const TTuple<FUniqueNetIdRepl, TTuple<FUserOnlineAccountAccelByte, int>>& UserInfo : CachedUsersInfo)
		{
			if (GameSessionOnlineSession->CompareAccelByteUniqueId(PlayerUniqueNetId, UserInfo.Key) &&
				UserInfo.Value.Value != INDEX_NONE)
			{
				UE_LOG_GAMESESSION(Log, TEXT("Found cache"));

				// cache found, trigger immediately
				AbPlayerState->SetPlayerName(UserInfo.Value.Key.GetDisplayName());
				AbPlayerState->TeamId = UserInfo.Value.Value;
				UserInfo.Value.Key.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AbPlayerState->AvatarURL);

				ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this, PlayerController]()
				{
					OnAuthenticatePlayerComplete(PlayerController, true);
				}));
				return;
			}
		}
	}

	if (QueryUserInfoFromSessionQueue.Contains(PlayerController))
	{
		UE_LOG_GAMESESSION(Verbose, TEXT("Player already in queue and have not go through all the sequence yet. Waiting for response"))
		return;
	}

	// does not present in cache, query session info
	AuthenticatePlayer_AddPlayerControllerToQueryQueue(PlayerController);
	return;
}

void UAccelByteWarsServerSubsystemBase::AuthenticatePlayer_AddPlayerControllerToQueryQueue(APlayerController* PlayerController)
{
	UE_LOG_GAMESESSION(Verbose, TEXT("called"))

	// check if target user already in query
	if (QueryUserInfoFromSessionQueue.Contains(PlayerController))
	{
		QueryUserInfoFromSessionAttemptLeft--;

		UE_LOG_GAMESESSION(Verbose, TEXT("Player already in queue, decreasing attempt to: %d"), QueryUserInfoFromSessionAttemptLeft);
	}
	else
	{
		QueryUserInfoFromSessionQueue.Add(PlayerController);
		QueryUserInfoFromSessionAttemptLeft = QueryUserInfoFromSessionAttemptLimit;

		UE_LOG_GAMESESSION(Verbose, TEXT("Player added to queue, resetting attempt to: %d"), QueryUserInfoFromSessionAttemptLeft);
	}

	// check if attempt left has been exhausted
	if (QueryUserInfoFromSessionAttemptLeft <= 0)
	{
		UE_LOG_GAMESESSION(Verbose, TEXT("Attempt exhausted: flag player as not in session"));

		AuthenticatePlayer_CompleteTask(false);
		return;
	}

	// only execute if the same task is not currently running
	if (!bQueryUserInfoFromSessionRunning && bHasReceivedSession)
	{
		bQueryUserInfoFromSessionRunning = 
			GetABSessionInt()->RefreshSession(
				GameSessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
				FOnRefreshSessionComplete::CreateUObject(this, &ThisClass::AuthenticatePlayer_OnRefreshSessionComplete));

		UE_LOG_GAMESESSION(Verbose, TEXT("RefreshSession: executed: %s"),
			*FString(bQueryUserInfoFromSessionRunning ? "TRUE" : "FALSE"));

		if (!bQueryUserInfoFromSessionRunning)
		{
			UE_LOG_GAMESESSION(Warning, TEXT("RefreshSession: canceled. Something went wrong"));
			AuthenticatePlayer_CompleteTask(false);
		}
	}
	else
	{
		UE_LOG_GAMESESSION(Verbose, TEXT("Task already running, waiting for response"));
	}
}

void UAccelByteWarsServerSubsystemBase::AuthenticatePlayer_OnRefreshSessionComplete(bool bSucceeded)
{
	UE_LOG_GAMESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
	{
		const FNamedOnlineSession* NamedOnlineSession =
			GameSessionOnlineSession->GetSession(
				GameSessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		if (!NamedOnlineSession)
		{
			return;
		}

		const TSharedPtr<FOnlineSessionInfo> SessionInfo = NamedOnlineSession->SessionInfo;
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

		TArray<FAccelByteModelsV2SessionUser> AbMembers = AbBaseSessionInfo->Members;

		TArray<FUniqueNetIdRef> UniqueNetIds;
		for (const FAccelByteModelsV2SessionUser& AbMember : AbMembers)
		{
			FAccelByteUniqueIdComposite CompositeId;
			CompositeId.Id = AbMember.ID;

			FUniqueNetIdAccelByteUserRef AccelByteUser = FUniqueNetIdAccelByteUser::Create(CompositeId);
			UniqueNetIds.Add(AccelByteUser);
		}

		// if query empty, skip process. Preventing multiple QueryUserInfo run simultaneously.
		if (QueryUserInfoFromSessionQueue.IsEmpty())
		{
			UE_LOG_GAMESESSION(Verbose, TEXT("Queue empty, ends sequence here."))
			return;
		}

		if (IsRunningDedicatedServer())
		{
			GameSessionOnlineSession->DSQueryUserInfo(
				UniqueNetIds,
				FOnDSQueryUsersInfoComplete::CreateUObject(this, &ThisClass::AuthenticatePlayer_OnDSQueryUserInfoComplete));
		}
		else
		{
			GameSessionOnlineSession->QueryUserInfo(
				0,
				UniqueNetIds,
				FOnQueryUsersInfoComplete::CreateUObject(this, &ThisClass::AuthenticatePlayer_OnQueryUserInfoComplete));
		}
	}
	else
	{
		AuthenticatePlayer_CompleteTask(false);
	}
}

void UAccelByteWarsServerSubsystemBase::AuthenticatePlayer_OnQueryUserInfoComplete(
	const bool bSucceeded,
	const TArray<FUserOnlineAccountAccelByte*>& OnlineUsers)
{
	UE_LOG_GAMESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	bQueryUserInfoFromSessionRunning = false;

	if (bSucceeded)
	{
		// cache data
		for (const FUserOnlineAccountAccelByte* OnlineUser : OnlineUsers)
		{
			TPair<FUserOnlineAccountAccelByte, int32>& UserInfo = CachedUsersInfo.FindOrAdd(FUniqueNetIdRepl(OnlineUser->GetUserId()));
			UserInfo.Key = *OnlineUser;
			UserInfo.Value = INDEX_NONE;
		}

		// update cache with team id
		UpdateUserCache();

		AuthenticatePlayer_CompleteTask(true);
	}
	else
	{
		AuthenticatePlayer_CompleteTask(false);
	}
}

void UAccelByteWarsServerSubsystemBase::AuthenticatePlayer_OnDSQueryUserInfoComplete(
	const bool bSucceeded,
	const TArray<const FBaseUserInfo*> UserInfos)
{
	UE_LOG_GAMESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	bQueryUserInfoFromSessionRunning = false;

	if (bSucceeded)
	{
		// cache data
		for (const FBaseUserInfo* User : UserInfos)
		{
			UE_LOG_GAMESESSION(Verbose, TEXT("cache added: %s"), *User->UserId);
			TPair<FBaseUserInfo, int32>& UserInfo = DSCachedUsersInfo.FindOrAdd(User->UserId);
			UserInfo.Key = *User;
			UserInfo.Value = INDEX_NONE;
		}

		// update cache with team id
		const FNamedOnlineSession* NamedOnlineSession =
			GameSessionOnlineSession->GetSession(
				GameSessionOnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
		if (!NamedOnlineSession)
		{
			UE_LOG_GAMESESSION(Warning, TEXT("Session is null"))
			return;
		}

		const TSharedPtr<FOnlineSessionInfo> SessionInfo = NamedOnlineSession->SessionInfo;
		if (!SessionInfo.IsValid())
		{
			UE_LOG_GAMESESSION(Warning, TEXT("Session Info is null"))
			return;
		}

		const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
		if (!AbSessionInfo.IsValid())
		{
			UE_LOG_GAMESESSION(Warning, TEXT("AB Session Info is null"))
			return;
		}

		TArray<FAccelByteModelsV2GameSessionTeam> Teams = AbSessionInfo->GetTeamAssignments();
		for (int i = 0; i < Teams.Num(); ++i)
		{
			for (const FString& UserId : Teams[i].UserIDs)
			{
				if (DSCachedUsersInfo.Contains(UserId))
				{
					UE_LOG_GAMESESSION(Verbose, TEXT("%s: team updated: %d"), *UserId, i)
					DSCachedUsersInfo[UserId].Value = i;
				}
			}
		}

		AuthenticatePlayer_CompleteTask(true);
	}
	else
	{
		AuthenticatePlayer_CompleteTask(false);
	}
}

void UAccelByteWarsServerSubsystemBase::AuthenticatePlayer_CompleteTask(const bool bSucceeded)
{
	UE_LOG_GAMESESSION(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	// update user info in PlayerController and clear retrieved user info from QueryArray
	for (APlayerController* PlayerController : QueryUserInfoFromSessionQueue)
	{
		if (!PlayerController)
		{
			PlayerController = nullptr;
			continue;
		}

		APlayerState* PlayerState = PlayerController->PlayerState;
		if (!PlayerState)
		{
			PlayerController = nullptr;
			continue;
		}

		AAccelByteWarsPlayerState* AbPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
		if (!AbPlayerState)
		{
			PlayerController = nullptr;
			continue;
		}

		// trigger delegate as fail if bSucceeded == false
		if (!bSucceeded)
		{
			UE_LOG_GAMESESSION(Warning, TEXT("Info not found, trigger player's delegate as failed"));

			OnAuthenticatePlayerComplete(PlayerController, false);
			PlayerController = nullptr;
			continue;
		}

		// update user's info
		bool bFound = false;
		if (IsRunningDedicatedServer())
		{
			const FUniqueNetIdAccelByteUserPtr UniqueNetId =
				FUniqueNetIdAccelByteUser::TryCast(PlayerState->GetUniqueId().GetUniqueNetId().ToSharedRef());
			if (DSCachedUsersInfo.Contains(UniqueNetId->GetAccelByteId()))
			{
				const TTuple<FBaseUserInfo, int>& UserInfo = DSCachedUsersInfo[UniqueNetId->GetAccelByteId()];
				AbPlayerState->SetPlayerName(UserInfo.Key.DisplayName);
				AbPlayerState->TeamId = UserInfo.Value;
				AbPlayerState->AvatarURL = UserInfo.Key.AvatarUrl;

				bFound = true;
			}
		}
		else
		{
			const FUniqueNetIdRepl& UniqueNetId = PlayerState->GetUniqueId();
			if (CachedUsersInfo.Contains(UniqueNetId))
			{
				const TTuple<FUserOnlineAccountAccelByte, int>& UserInfo = CachedUsersInfo[UniqueNetId];
				AbPlayerState->SetPlayerName(UserInfo.Key.GetDisplayName());
				AbPlayerState->TeamId = UserInfo.Value;
				UserInfo.Key.GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AbPlayerState->AvatarURL);

				bFound = true;
			}
		}

		if (AbPlayerState->TeamId != INDEX_NONE && bFound)
		{
			UE_LOG_GAMESESSION(Verbose, TEXT("Trigger player's delegate as succeeded"));

			// trigger delegate
			OnAuthenticatePlayerComplete(PlayerController, true);

			PlayerController = nullptr;
		}
		else
		{
			UE_LOG_GAMESESSION(Verbose, TEXT("Info not found, re-attempt RefreshSession"));

			// re-trigger sequence
			AuthenticatePlayer_AddPlayerControllerToQueryQueue(PlayerController);
		}
	}

	// remove nullptr
	QueryUserInfoFromSessionQueue.RemoveAll([](const APlayerController* Element)
	{
		UE_LOG_GAMESESSION(Verbose, TEXT("Removing player from queue"));
		return !Element;
	});
}

void UAccelByteWarsServerSubsystemBase::OnAuthenticatePlayerComplete(
	APlayerController* PlayerController,
	bool bPlayerIsInSession)
{
	UE_LOG_GAMESESSION(Verbose, TEXT("called"))

	// Get GameMode
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameModeBase* GameMode = World->GetAuthGameMode();
	if (!GameMode)
	{
		return;
	}

	AAccelByteWarsGameMode* AbGameMode = Cast<AAccelByteWarsGameMode>(GameMode);
	if (!AbGameMode)
	{
		return;
	}

	// Get PlayerState
	if (!PlayerController)
	{
		return;
	}

	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!PlayerState)
	{
		GameMode->GameSession->KickPlayer(PlayerController, FText::FromString("Invalid PlayerState"));
		return;
	}

	const AAccelByteWarsPlayerState* AbPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
	if (!AbPlayerState)
	{
		GameMode->GameSession->KickPlayer(PlayerController, FText::FromString("Invalid PlayerState class"));
		return;
	}

	// kick if bSucceeded == false or assigned team id == INDEX_NONE
	if (!bPlayerIsInSession || AbPlayerState->TeamId == INDEX_NONE)
	{
		UE_LOG_GAMESESSION(Warning, TEXT("Info does not exist in session info, kicking player"));

		// treat as not a part of the session
		GameMode->GameSession->KickPlayer(PlayerController, FText::FromString("Invalid PlayerState class"));
	}
	else
	{
		UE_LOG_GAMESESSION(Log, TEXT("success"));

		/**
		 * Give an opportunity for other modules to modify player's data.
		 * At this stage, player data exist in session and player's info have been retrieved.
		 */
		OnAuthenticatePlayerComplete_PrePlayerSetup(PlayerController);

		AbGameMode->DelayedPlayerTeamSetupWithPredefinedData(PlayerController);
	}
}
#pragma endregion 
