// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchmakingDSServerSubsystem.h"

#include "MatchmakingDSLog.h"
#include "OnlineSubsystemAccelByteSessionSettings.h"
#include "Core/GameStates/AccelByteWarsGameState.h"

void UMatchmakingDSServerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	GetABSessionInt()->OnServerReceivedSessionDelegates.RemoveAll(this);
}

void UMatchmakingDSServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	MatchmakingOnlineSession = Cast<UMatchmakingDSOnlineSession>(BaseOnlineSession);

	GetABSessionInt()->OnServerReceivedSessionDelegates.AddUObject(this, &ThisClass::OnServerSessionReceived);
}

void UMatchmakingDSServerSubsystem::RegisterServer(const FName SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// safety
	if (!GetABSessionInt())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
		{
			OnRegisterServerComplete(false);
		}));
		return;
	}

	if (!IsRunningDedicatedServer())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Is not DS"));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
		{
			OnRegisterServerComplete(false);
		}));
		return;
	}

	if (bServerAlreadyRegister)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Already registered"));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
		{
			OnRegisterServerComplete(false);
		}));
		return;
	}

	GetABSessionInt()->RegisterServer(SessionName, FOnRegisterServerComplete::CreateUObject(
		this, &ThisClass::OnRegisterServerComplete));
}

void UMatchmakingDSServerSubsystem::UnregisterServer(const FName SessionName)
{
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

	// safety
	if (!GetABSessionInt())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session interface null"))
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
		{
			OnUnregisterServerComplete(false);
		}));
		return;
	}

	if (!IsRunningDedicatedServer())
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Is not DS"));
		ExecuteNextTick(FSimpleDelegate::CreateWeakLambda(this, [this]()
		{
			OnUnregisterServerComplete(false);
		}));
		return;
	}

	GetABSessionInt()->UnregisterServer(SessionName, FOnUnregisterServerComplete::CreateUObject(
		this, &ThisClass::OnUnregisterServerComplete));
	bUnregisterServerRunning = true;
}

void UMatchmakingDSServerSubsystem::OnRegisterServerComplete(bool bSucceeded)
{
	UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	if (bSucceeded)
	{
		bServerAlreadyRegister = true;
	}
}

void UMatchmakingDSServerSubsystem::OnUnregisterServerComplete(bool bSucceeded)
{
	UE_LOG_MATCHMAKINGDS(Log, TEXT("succeeded: %s"), *FString(bSucceeded ? "TRUE": "FALSE"))

	bUnregisterServerRunning = false;

	FPlatformMisc::RequestExit(false);
}

void UMatchmakingDSServerSubsystem::OnServerSessionReceived(FName SessionName)
{
	Super::OnServerSessionReceived(SessionName);
	UE_LOG_MATCHMAKINGDS(Verbose, TEXT("called"))

#pragma region "Assign game mode based on SessionTemplateName from backend"
	// Get GameMode
	const UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("World is invalid"));
		return;
	}

	AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Game State is invalid"));
		return;
	}

	AAccelByteWarsGameState* AbGameState = Cast<AAccelByteWarsGameState>(GameState);
	if (!AbGameState)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Game State is not derived from AAccelByteWarsGameState"));
		return;
	}

	// Get Game Session
	if (MatchmakingOnlineSession->GetSessionType(SessionName) != EAccelByteV2SessionType::GameSession)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Is not a game session"));
		return;
	}

	const FNamedOnlineSession* Session = MatchmakingOnlineSession->GetSession(SessionName);
	if (!Session)
	{
		UE_LOG_MATCHMAKINGDS(Warning, TEXT("Session is invalid"));
		return;
	}

	FString SessionTemplateName;
	Session->SessionSettings.Get(SETTING_SESSION_MATCHPOOL, SessionTemplateName);
	if (!SessionTemplateName.IsEmpty())
	{
		AbGameState->AssignGameMode(MatchmakingOnlineSession->TargetGameModeMap[SessionTemplateName]);
	}
#pragma endregion

	// Query all currently registered user's info
	AuthenticatePlayer_OnRefreshSessionComplete(true);
}
