// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameModes/AccelByteWarsMainMenuGameMode.h"

#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

void AAccelByteWarsMainMenuGameMode::InitGameState()
{
	Super::InitGameState();

	ABMainMenuGameState = GetGameState<AAccelByteWarsMainMenuGameState>();

	if (!ensure(ABMainMenuGameState))
	{
		GAMEMODE_LOG(Warning, TEXT("Game State is not (derived from) AAccelByteWarsMainMenuGameStateBase"));
		return;
	}
}

void AAccelByteWarsMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	/**
	 * Remove additional local players
	 * Removing player while iterating through PlayerArray ocasionally result in "Array has changed during ranged-for iteration"
	 * Workaround: duplicate PlayerController ptr to be removed
	 */
	TArray<APlayerController*> PlayerToBeRemoved;
	for (const APlayerState* PlayerState : ABMainMenuGameState->PlayerArray)
	{
		APlayerController* PC = PlayerState->GetPlayerController();
		if (!PC)
		{
			continue;
		}

		// if first controller, skip
		const int32 ControllerId = UGameplayStatics::GetPlayerControllerID(PC);
		if (ControllerId == 0)
		{
			continue;
		}

		PlayerToBeRemoved.Add(PC);
	}
	// remove players
	for (APlayerController* PlayerController : PlayerToBeRemoved)
	{
		UGameplayStatics::RemovePlayer(PlayerController, true);
	}
}

APlayerController* AAccelByteWarsMainMenuGameMode::Login(
	UPlayer* NewPlayer,
	ENetRole InRemoteRole,
	const FString& Portal,
	const FString& Options,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	APlayerController* PlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

#pragma region "Lobby Countdown Implementation"
	if (IsServer())
	{
		// Start lobby countdown.
		if (!bIsGameplayLevel && ABMainMenuGameState->LobbyStatus == ELobbyStatus::IDLE)
		{
			ABMainMenuGameState->LobbyStatus = ELobbyStatus::LOBBY_COUNTDOWN_STARTED;
		}
	}
#pragma endregion

	return PlayerController;
}

void AAccelByteWarsMainMenuGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#pragma region "Lobby Countdown Implementation"
	// Run on server
	if (IsServer())
	{
		switch (ABMainMenuGameState->LobbyStatus)
		{
		case ELobbyStatus::LOBBY_COUNTDOWN_STARTED:
			// only countdown if on DS
			if (IsRunningDedicatedServer())
			{
				if (ABMainMenuGameState->LobbyCountdown <= 0)
				{
					ABMainMenuGameState->LobbyStatus = ELobbyStatus::GAME_STARTED;
				}
				else
				{
					ABMainMenuGameState->LobbyCountdown -= DeltaSeconds;
				}
			}
			break;
		case ELobbyStatus::GAME_STARTED:
			// trigger server travel
			if (!ABMainMenuGameState->bIsServerTravelling)
			{
				DelayedServerTravel("/Game/ByteWars/Maps/GalaxyWorld/GalaxyWorld");
			}
			break;
		default: ;
		}
	}
#pragma endregion 
}

void AAccelByteWarsMainMenuGameMode::CreateLocalGameSetup(const FString& CodeName, const int32 LocalPlayerNum)
{
	ABMainMenuGameState->AssignGameMode(CodeName);

	// setup local player
	for (int i = 0; i < LocalPlayerNum; ++i)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerControllerFromID(GetWorld(), i);
		if (!PC)
		{
			PC = UGameplayStatics::CreatePlayer(GetWorld(), i);
		}

		PlayerTeamSetup(PC);
	}
}