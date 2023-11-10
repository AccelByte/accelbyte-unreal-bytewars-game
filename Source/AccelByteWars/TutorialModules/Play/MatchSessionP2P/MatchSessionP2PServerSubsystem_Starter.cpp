// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MatchSessionP2PServerSubsystem_Starter.h"

#include "MatchSessionP2PLog.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/Player/AccelByteWarsPlayerState.h"

#pragma region "Game specific"
void UMatchSessionP2PServerSubsystem_Starter::OnAuthenticatePlayerComplete_PrePlayerSetup(
	APlayerController* PlayerController)
{
	Super::OnAuthenticatePlayerComplete_PrePlayerSetup(PlayerController);

	// get GameMode
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!ensure(GameMode))
	{
		return;
	}
	const AAccelByteWarsGameMode* AbGameMode = Cast<AAccelByteWarsGameMode>(GameMode);
	if (!ensure(AbGameMode))
	{
		return;
	}

	// get PlayerState
	APlayerState* PlayerState = PlayerController->PlayerState;
	if (!ensure(PlayerState))
	{
		return;
	}
	AAccelByteWarsPlayerState* AbPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerState);
	if (!ensure(AbPlayerState))
	{
		return;
	}

	AbGameMode->AssignTeamManually(AbPlayerState->TeamId);
}
#pragma endregion 

void UMatchSessionP2PServerSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UOnlineSession* BaseOnlineSession = GetWorld()->GetGameInstance()->GetOnlineSession();
	if (!ensure(BaseOnlineSession))
	{
		return;
	}
	OnlineSession = Cast<UMatchSessionP2POnlineSession>(BaseOnlineSession);

	// TODO: Add your delegate setup here
}

void UMatchSessionP2PServerSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Add your delegate cleanup here
}

#pragma region "Matchmaking with P2P implementations"
// TODO: Add your module implementations here.
#pragma endregion 
