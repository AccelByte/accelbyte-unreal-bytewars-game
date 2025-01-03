// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "CustomMatchSubsystem.h"

#include "CustomMatchLog.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UCustomMatchSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get online session
	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		UE_LOG_CUSTOMMATCH(Warning, TEXT("Online Session (UOnlineSession) invalid"))
		return;
	}
	OnlineSession = Cast<UAccelByteWarsOnlineSessionBase>(BaseOnlineSession);
	if (!ensure(OnlineSession))
	{
		UE_LOG_CUSTOMMATCH(Warning, TEXT("Online Session (UAccelByteWarsOnlineSessionBase) invalid"))
		return;
	}

	// Response delegates setup
	OnlineSession->GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreateSessionComplete);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnLeaveGameSessionComplete);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->AddUObject(this, &ThisClass::OnSessionServerUpdateReceived);
}

void UCustomMatchSubsystem::Deinitialize()
{
	Super::Deinitialize();

	// Response delegates cleanup
	OnlineSession->GetOnCreateSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnLeaveSessionCompleteDelegates()->RemoveAll(this);
	OnlineSession->GetOnSessionServerUpdateReceivedDelegates()->RemoveAll(this);
}

void UCustomMatchSubsystem::CreateCustomGameSession(
	const int32 LocalUserNum,
	const EGameModeNetworkType NetworkType,
	const EGameModeType GameModeType,
	const EAccelByteV2SessionJoinability Joinability,
	const int32 Duration,
	const int32 PlayerLives,
	const int32 MissileLimit /* -1 = unlimited*/,
	const int32 MaxPlayerTotalNum,
	const int32 MaxTeamNum)
{
	FOnlineSessionSettings SessionSettings;

#pragma region "Session Settings setup"
	EAccelByteV2SessionConfigurationServerType ServerType;
	switch (NetworkType)
	{
	case EGameModeNetworkType::DS:
		ServerType = EAccelByteV2SessionConfigurationServerType::DS;
		break;
	case EGameModeNetworkType::P2P:
		ServerType = EAccelByteV2SessionConfigurationServerType::P2P;
		break;
	default:
		ServerType = EAccelByteV2SessionConfigurationServerType::NONE;
	}

	// BE specific settings | will be used by BE
	SessionSettings.Set(SETTING_SESSION_SERVER_TYPE, UEnum::GetValueAsString(ServerType));
	SessionSettings.Set(SETTING_SESSION_JOIN_TYPE, UEnum::GetValueAsString(Joinability));
	if (Joinability == EAccelByteV2SessionJoinability::OPEN)
	{
		SessionSettings.NumPublicConnections = MaxPlayerTotalNum;
	}
	else
	{
		SessionSettings.NumPrivateConnections = MaxPlayerTotalNum;
	}

	// game specific settings | will be used by server
	SessionSettings.Set(GAMESETUP_IsCustomGame, true);
	SessionSettings.Set(GAMESETUP_DisplayName, TEXT("Custom Game"));
	SessionSettings.Set(GAMESETUP_GameModeType, UEnum::GetValueAsString(GameModeType));
	SessionSettings.Set(GAMESETUP_NetworkType, UEnum::GetValueAsString(NetworkType));
	SessionSettings.Set(GAMESETUP_MatchTime, static_cast<double>(Duration));
	SessionSettings.Set(GAMESETUP_StartingLives, static_cast<double>(PlayerLives));
	SessionSettings.Set(GAMESETUP_FiredMissilesLimit, static_cast<double>(MissileLimit));
	SessionSettings.Set(GAMESETUP_MaxPlayers, static_cast<double>(MaxPlayerTotalNum));
	SessionSettings.Set(GAMESETUP_MaxTeamNum, static_cast<double>(GameModeType == EGameModeType::TDM ? MaxTeamNum : MaxPlayerTotalNum));
	SessionSettings.Set(GAMESETUP_IsTeamGame, GameModeType == EGameModeType::TDM);
	SessionSettings.Set(GAMESETUP_StartGameCountdown, static_cast<double>(INDEX_NONE));

	// Set a flag so we can request a filtered session from backend.
	// We're using the Match Session flag as we want the browse matches to work with this right away
	SessionSettings.Set(GAME_SESSION_REQUEST_TYPE, GAME_SESSION_REQUEST_TYPE_MATCHSESSION);
#pragma endregion

	FString MatchTemplateName = FString::Printf(TEXT("unreal-%s-%s"),
		GameModeType == EGameModeType::FFA ? TEXT("elimination") : TEXT("teamdeathmatch"),
		NetworkType == EGameModeNetworkType::DS ? TEXT("ds-ams") : TEXT("p2p"));

	// Override match session template name if applicable.
	if (NetworkType == EGameModeNetworkType::DS && !UTutorialModuleOnlineUtility::GetMatchSessionTemplateDSOverride().IsEmpty())
	{
		MatchTemplateName = UTutorialModuleOnlineUtility::GetMatchSessionTemplateDSOverride();
	}
	else if (NetworkType == EGameModeNetworkType::P2P && !UTutorialModuleOnlineUtility::GetMatchSessionTemplateP2POverride().IsEmpty())
	{
		MatchTemplateName = UTutorialModuleOnlineUtility::GetMatchSessionTemplateP2POverride();
	}

	OnlineSession->GetOnCreateSessionCompleteDelegates()->AddUObject(this, &ThisClass::OnCreateSessionComplete);
	OnlineSession->CreateSession(
		LocalUserNum,
		OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession),
		SessionSettings,
		EAccelByteV2SessionType::GameSession,
		MatchTemplateName);
}

void UCustomMatchSubsystem::LeaveGameSession() const
{
	OnlineSession->LeaveSession(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession));
}

void UCustomMatchSubsystem::OnCreateSessionComplete(const FName SessionName, const bool bSucceeded) const
{
	// don't execute if not game session
	if (OnlineSession->GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		return;
	}

	OnCreateCustomGameSessionCompleteDelegates.Broadcast(SessionName, bSucceeded);
}

void UCustomMatchSubsystem::OnLeaveGameSessionComplete(const FName SessionName, const bool bSucceeded) const
{
	// don't execute if not game session
	// in this case, the OnlineSession will be empty, hence compare directly using session name 
	if (!SessionName.IsEqual(OnlineSession->GetPredefinedSessionNameFromType(EAccelByteV2SessionType::GameSession)))
	{
		return;
	}

	OnLeaveGameSessionDelegates.Broadcast(SessionName, bSucceeded);
}

void UCustomMatchSubsystem::OnSessionServerUpdateReceived(
	const FName SessionName,
	const FOnlineError& Error,
	const bool bHasClientTravelTriggered) const
{
	// don't execute if not game session
	if (OnlineSession->GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		return;
	}

	OnSessionServerUpdateReceivedDelegates.Broadcast(SessionName, Error, bHasClientTravelTriggered);
}
