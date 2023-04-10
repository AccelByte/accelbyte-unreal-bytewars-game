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

	// Remove additional local players
	for (int i = 0; i < ABMainMenuGameState->PlayerArray.Num(); ++i)
	{
		if (!ABMainMenuGameState->PlayerArray.IsValidIndex(i))
		{
			ABMainMenuGameState->PlayerArray.RemoveAt(i);
			continue;
		}
		const APlayerState* PS = ABMainMenuGameState->PlayerArray[i];

		APlayerController* PC = PS->GetPlayerController();
		if (!PC)
		{
			continue;
		}

		// if first controller, skip
		if (UGameplayStatics::GetPlayerControllerID(PC) == 0)
		{
			continue;
		}

		if (HasAuthority())
		{
			UGameplayStatics::RemovePlayer(PC, true);
		}
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
