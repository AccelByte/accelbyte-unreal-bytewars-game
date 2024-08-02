// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"

#include "Net/UnrealNetwork.h"

#define LOBBY_SHUTDOWN_COUNTDOWN_PARAM TEXT("LOBBY_SHUTDOWN_COUNTDOWN")

void AAccelByteWarsMainMenuGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, LobbyStatus);
	DOREPLIFETIME(ThisClass, LobbyCountdown);
	DOREPLIFETIME(ThisClass, LobbyShutdownCountdown);
}

float AAccelByteWarsMainMenuGameState::GetLobbyShutdownCountdown() const
{
	return LobbyShutdownCountdown;
}

void AAccelByteWarsMainMenuGameState::ReduceLobbyShutdownCountdown(const float DeltaSeconds)
{
	LobbyShutdownCountdown -= DeltaSeconds;
}

void AAccelByteWarsMainMenuGameState::ResetLobbyShutdownCountdown()
{
	// Get from launch args first
	FString LobbyShutdownCountdownString = TEXT("");
	const FString LobbyShutdownCountdownCmd = FString::Printf(TEXT("-%s="), LOBBY_SHUTDOWN_COUNTDOWN_PARAM);
	FParse::Value(FCommandLine::Get(), *LobbyShutdownCountdownCmd, LobbyShutdownCountdownString);
	if (!LobbyShutdownCountdownString.IsEmpty())
	{
		LobbyShutdownCountdown = FCString::Atof(*LobbyShutdownCountdownString);
		return;
	}

	// Get from game setup
	LobbyShutdownCountdown = GameSetup.NotEnoughPlayerShutdownLobbyCountdown;
}

void AAccelByteWarsMainMenuGameState::BeginPlay()
{
	Super::BeginPlay();

	ResetLobbyShutdownCountdown();
}

#undef LOBBY_SHUTDOWN_COUNTDOWN_PARAM