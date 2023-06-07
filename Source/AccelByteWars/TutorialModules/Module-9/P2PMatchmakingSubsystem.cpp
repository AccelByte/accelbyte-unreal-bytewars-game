// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-9/P2PMatchmakingSubsystem.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/System/AccelByteWarsGameSession.h"

void UP2PMatchmakingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Let the parent class handle server registration instead, since it can only done one at time.
	AAccelByteWarsGameSession::OnRegisterServerDelegate.RemoveAll(this);

	/* When transitioning to the listen server (P2P server), the server will initialize
	 * a new Game Instance and Game State (this is Unreal Engine's default behavior).
	 * Thus, the data set up before transitioning to the listen server will be reset.
	 * Therefore, the delegate below will help to reinitialize the listen server data. */
	AAccelByteWarsGameMode::OnInitializeListenServer.AddUObject(this, &ThisClass::OnServerReceivedSession);
}

void UP2PMatchmakingSubsystem::Deinitialize()
{
	Super::Deinitialize();

	AAccelByteWarsGameMode::OnInitializeListenServer.RemoveAll(this);
}

bool UP2PMatchmakingSubsystem::TravelClient(FName SessionName, APlayerController* PC)
{
	if (!IsGameSessionValid(SessionName))
	{
		return false;
	}

	if (!ensure(SessionInterface.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the host server. Session Interface is not valid."));
		return false;
	}

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(SessionName);
	if (!ensure(Session))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the host server. Session is not null."));
		return false;
	}

	TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!ensure(SessionInfo.IsValid()))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the host server. Session Info is not valid."));
		return false;
	}

	if (!ensure(PC))
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the host server. PlayerController is not valid."));
		return false;
	}

	FString HostUrl{};
	if (SessionInterface->GetResolvedConnectString(SessionName, HostUrl) && !HostUrl.IsEmpty())
	{
		// If the game client is the host, then travel as listen server.
		if (SessionInterface->IsPlayerP2PHost(GetPlayerUniqueNetId(PC).ToSharedRef().Get(), SessionName)) 
		{
			PC->ClientTravel(TEXT("MainMenu?listen"), TRAVEL_Absolute);
		}
		// If not, then travel to the host.
		else
		{
			PC->ClientTravel(HostUrl, TRAVEL_Absolute);
		}
		return true;
	}
	else
	{
		UE_LOG_MATCHMAKING_ESSENTIALS(Warning, TEXT("Cannot travel the client to the host server. Host URL is not valid."));
		return false;
	}
}