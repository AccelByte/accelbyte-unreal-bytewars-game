// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchSessionDSOnlineSession.h"

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

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-RegisterOnlineDelegates
// @@@MULTISNIP BindFindSessionDelegate {"selectedLines": ["1-2", "19-23"]}
void UMatchSessionDSOnlineSession::RegisterOnlineDelegates()
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

	// Game Session delegates
	GetABSessionInt()->OnSessionServerUpdateDelegates.AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
	GetABSessionInt()->OnSessionServerErrorDelegates.AddUObject(this, &ThisClass::OnSessionServerErrorReceived);

	// Match session delegates
	GetSessionInt()->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::OnFindSessionsComplete);

	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-ClearOnlineDelegates
// @@@MULTISNIP UnbindFindSessionDelegate {"selectedLines": ["1-2", "12-13"]}
void UMatchSessionDSOnlineSession::ClearOnlineDelegates()
{
	Super::ClearOnlineDelegates();

	UPauseWidget::OnQuitGameDelegate.RemoveAll(this);
	UMatchLobbyWidget::OnQuitLobbyDelegate.RemoveAll(this);
	UGameOverWidget::OnQuitGameDelegate.RemoveAll(this);

	GetABSessionInt()->OnSessionServerUpdateDelegates.RemoveAll(this);
	GetABSessionInt()->OnSessionServerErrorDelegates.RemoveAll(this);

	GetSessionInt()->OnFindSessionsCompleteDelegates.RemoveAll(this);
}
// @@@SNIPEND

#pragma region "Game Session Essentials"
void UMatchSessionDSOnlineSession::DSQueryUserInfo(
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
		// Gather user IDs.
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

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-TravelToSession
bool UMatchSessionDSOnlineSession::TravelToSession(const FName SessionName)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	if (GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Not a game session"));
		return false;
	}

	// Get session info
	const FNamedOnlineSession* Session = GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("The session is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfo> SessionInfo = Session->SessionInfo;
	if (!SessionInfo.IsValid())
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("The session info is invalid"));
		return false;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> AbSessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(SessionInfo);
	if (!AbSessionInfo.IsValid())
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("The session info is not FOnlineSessionInfoAccelByteV2"));
		return false;
	}

	// get player controller of the local owner of the user
	APlayerController* PlayerController = GetPlayerControllerByUniqueNetId(Session->LocalOwnerId);

	// if nullptr, treat as failed
	if (!PlayerController)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Can't find player controller with the session's local owner's unique ID"));
		return false;
	}

	AAccelByteWarsPlayerController* AbPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AbPlayerController)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Player controller is not (derived from) AAccelByteWarsPlayerController"));
		return false;
	}

	// Make sure this is not a P2P session
	if (GetABSessionInt()->IsPlayerP2PHost(GetLocalPlayerUniqueNetId(PlayerController).ToSharedRef().Get(), SessionName)) 
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("The session is a P2P session"));
		return false;
	}
	
	FString ServerAddress = "";
	GetABSessionInt()->GetResolvedConnectString(SessionName, ServerAddress);

	if (ServerAddress.IsEmpty())
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Can't find session's server address"));
		return false;
	}

	if (!bIsInSessionServer)
	{
		AbPlayerController->DelayedClientTravel(ServerAddress, TRAVEL_Absolute);
		bIsInSessionServer = true;
	}
	else
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Already in session's server"));
	}

	return true;
}
// @@@SNIPEND

void UMatchSessionDSOnlineSession::OnDSQueryUserInfoComplete(
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

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-OnSessionServerUpdateReceived
void UMatchSessionDSOnlineSession::OnSessionServerUpdateReceived(FName SessionName)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	if (bLeavingSession)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("called but leave session is currently running. Canceling attempt to travel to server"))
		OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), false);
		return;
	}

	const bool bHasClientTravelTriggered = TravelToSession(SessionName);
	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, FOnlineError(true), bHasClientTravelTriggered);
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-OnSessionServerErrorReceived
void UMatchSessionDSOnlineSession::OnSessionServerErrorReceived(FName SessionName, const FString& Message)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	FOnlineError Error;
	Error.bSucceeded = false;
	Error.ErrorMessage = FText::FromString(Message);

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, false);
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-HandleDisconnectInternal
bool UMatchSessionDSOnlineSession::HandleDisconnectInternal(UWorld* World, UNetDriver* NetDriver)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	LeaveSession(GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
	bIsInSessionServer = false;

	GEngine->HandleDisconnect(World, NetDriver);

	return true;
}
// @@@SNIPEND
#pragma endregion

#pragma region "Match Session Essentials"
// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-CreateMatchSession
void UMatchSessionDSOnlineSession::CreateMatchSession(
	const int32 LocalUserNum,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType)
{
	FOnlineSessionSettings SessionSettings;
	// Set a flag so we can request a filtered session from backend
	SessionSettings.Set(GAME_SESSION_REQUEST_TYPE, GAME_SESSION_REQUEST_TYPE_MATCHSESSION);

	// flag to signify the server which game mode to use
	SessionSettings.Set(
		GAMESETUP_GameModeCode,
		FString(GameModeType == EGameModeType::FFA ? "ELIMINATION-DS-USERCREATED" : "TEAMDEATHMATCH-DS-USERCREATED"));
	
	// Get match session template name based on game mode type
	FString MatchTemplateName = MatchSessionTemplateNameMap[{EGameModeNetworkType::DS, GameModeType}];

	// AMS is the default multiplayer server on AGS. If the game runs on legacy AGS Armada, remove the -ams suffix.
	if(!UTutorialModuleOnlineUtility::GetIsServerUseAMS())
	{
		MatchTemplateName = MatchTemplateName.Replace(TEXT("-ams"), TEXT(""));
	}
	
	// Override match session template name if applicable.
	if (!UTutorialModuleOnlineUtility::GetMatchSessionTemplateDSOverride().IsEmpty())
	{
		MatchTemplateName = UTutorialModuleOnlineUtility::GetMatchSessionTemplateDSOverride();
	}

#pragma region "Region Preferences"
	// include region preferences into session setting
	if(NetworkType == EGameModeNetworkType::DS)
	{
		if (const UTutorialModuleDataAsset* ModuleDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
			FPrimaryAssetId{ "TutorialModule:REGIONPREFERENCES" },
			this,
			true))
		{
			const UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
			ensure(GameInstance);

			// Get enabled regions
			TArray<FString> EnabledRegions = {};

			if (ModuleDataAsset->IsStarterModeActive())
			{
				// Starter mode is active, use the starter subsystem
				URegionPreferencesSubsystem_Starter* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem_Starter>();
				if(RegionPreferencesSubsystem != nullptr)
				{
					EnabledRegions = RegionPreferencesSubsystem->GetEnabledRegion();
				}
			}
			else
			{
				// Starter mode is not active, use the non starter subsystem.
				URegionPreferencesSubsystem* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
				if(RegionPreferencesSubsystem != nullptr)
				{
					EnabledRegions = RegionPreferencesSubsystem->GetEnabledRegion();
				}
			}

			if(!EnabledRegions.IsEmpty())
			{
				FOnlineSessionSettingsAccelByte::Set(SessionSettings, SETTING_GAMESESSION_REQUESTEDREGIONS, EnabledRegions);
			}
		}
	}
#pragma endregion 

	CreateSession(
		LocalUserNum,
		GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		SessionSettings,
		EAccelByteV2SessionType::GameSession,
		MatchTemplateName);
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-FindSessions
void UMatchSessionDSOnlineSession::FindSessions(
	const int32 LocalUserNum,
	const int32 MaxQueryNum,
	const bool bForce)
{
	UE_LOG_MATCHSESSIONDS(Verbose, TEXT("called"))

	if (SessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
	{
		UE_LOG_MATCHSESSIONDS(Warning, TEXT("Currently searching"))
		return;
	}

	// check cache
	if (!bForce && MaxQueryNum <= SessionSearch->SearchResults.Num())
	{
		UE_LOG_MATCHSESSIONDS(Log, TEXT("Cache found"))

		// return cache
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnFindSessionsComplete(true);
		}));
		return;
	}

	SessionSearch->SearchState = EOnlineAsyncTaskState::NotStarted;
	SessionSearch->MaxSearchResults = MaxQueryNum;
	SessionSearch->SearchResults.Empty();
	LocalUserNumSearching = LocalUserNum;

	// reset
	SessionSearch->QuerySettings = FOnlineSearchSettings();

	// Request a filtered session from backend based on the flag we set on CreateSession_Caller
	SessionSearch->QuerySettings.Set(
		GAME_SESSION_REQUEST_TYPE, GAME_SESSION_REQUEST_TYPE_MATCHSESSION, EOnlineComparisonOp::Equals);

	if (!GetSessionInt()->FindSessions(LocalUserNum, SessionSearch))
	{
		ExecuteNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			OnFindSessionsComplete(false);
		}));
	}
}
// @@@SNIPEND

// @@@SNIPSTART MatchSessionDSOnlineSession.cpp-OnFindSessionsComplete
void UMatchSessionDSOnlineSession::OnFindSessionsComplete(bool bSucceeded)
{
	UE_LOG_MATCHSESSIONDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? TEXT("TRUE") : TEXT("FALSE")))

	if (bSucceeded)
	{
		// Remove owned session from result if exists
		const FUniqueNetIdPtr LocalUserNetId = GetIdentityInt()->GetUniquePlayerId(LocalUserNumSearching);
		SessionSearch->SearchResults.RemoveAll([this, LocalUserNetId](const FOnlineSessionSearchResult& Element)
		{
			return CompareAccelByteUniqueId(
				FUniqueNetIdRepl(LocalUserNetId),
				FUniqueNetIdRepl(Element.Session.OwningUserId));
		});

#pragma region "Region Preferences"
		// remove session that not in region preferences
		if (const UTutorialModuleDataAsset* ModuleDataAsset = UTutorialModuleUtility::GetTutorialModuleDataAsset(
			FPrimaryAssetId{ "TutorialModule:REGIONPREFERENCES" },
			this,
			true))
		{
			const UAccelByteWarsGameInstance* GameInstance = StaticCast<UAccelByteWarsGameInstance*>(GetGameInstance());
			ensure(GameInstance);

			if (ModuleDataAsset->IsStarterModeActive())
			{
				// Starter is enabled, use the starter subsystem.
				URegionPreferencesSubsystem_Starter* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem_Starter>();
				if(RegionPreferencesSubsystem != nullptr)
				{
					RegionPreferencesSubsystem->FilterSessionSearch(SessionSearch);
				}
			}
			else
			{
				// Starter is not enabled, use the non starter subsystem.
				URegionPreferencesSubsystem* RegionPreferencesSubsystem = GameInstance->GetSubsystem<URegionPreferencesSubsystem>();
				if(RegionPreferencesSubsystem != nullptr)
				{
					RegionPreferencesSubsystem->FilterSessionSearch(SessionSearch);
				}
			}
		}
#pragma endregion 

		// Get owner’s user info for queried user info.
		TArray<FUniqueNetIdRef> UserIds;
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UserIds.AddUnique(SearchResult.Session.OwningUserId->AsShared());
		}

		// Trigger to query user info
		if (UStartupSubsystem* StartupSubsystem = GetWorld()->GetGameInstance<UStartupSubsystem>())
		{
			StartupSubsystem->QueryUserInfo(
				LocalUserNumSearching,
				UserIds,
				FOnQueryUsersInfoCompleteDelegate::CreateUObject(this, &ThisClass::OnQueryUserInfoForFindSessionComplete));

			return;
		}
	}

	OnFindSessionsCompleteDelegates.Broadcast({}, false);
}
// @@@SNIPEND

void UMatchSessionDSOnlineSession::OnQueryUserInfoForFindSessionComplete(
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
#pragma endregion