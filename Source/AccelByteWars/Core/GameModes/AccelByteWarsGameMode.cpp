// Copyright Epic Games, Inc. All Rights Reserved.


#include "AccelByteWarsGameMode.h"

#include "Core/Player/AccelByteWarsPlayerState.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/System/AccelByteWarsGlobals.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsGameMode);

AAccelByteWarsGameMode::AAccelByteWarsGameMode()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f;
	bAllowTickBeforeBeginPlay = false;
}

void AAccelByteWarsGameMode::InitGameState()
{
	Super::InitGameState();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ABGameState = GetGameState<AAccelByteWarsGameState>();

	if (!GameInstance)
	{
		GAMEMODE_LOG(Warning, TEXT("Game Instance is not (derived from) UAccelByteWarsGameInstance"));
		return;
	}
	if (!ABGameState)
	{
		GAMEMODE_LOG(Warning, TEXT("Game State is not (derived from) AAccelByteWarsGameStateBase"));
		return;
	}
}

void AAccelByteWarsGameMode::BeginPlay()
{
	// Check if GameSetup have already been set up or not
	if (!ABGameState->GameSetup)
	{
		FString CodeName;
		FParse::Value(FCommandLine::Get(), TEXT("-GameMode="), CodeName);

		// if launch argument does not exist, will use the first game mode
		ABGameState->AssignGameMode(CodeName);

		/* When transitioning to the listen server (P2P server), the server will initialize 
		 * a new Game Instance and Game State (this is Unreal Engine's default behavior).
		 * Thus, the data set up before transitioning to the listen server will be reset.
		 * Therefore, the delegate below will help to reinitialize the listen server data 
		 * (e.g. assigning game mode through the Game State). */
		if (GetNetMode() == ENetMode::NM_ListenServer && OnInitializeListenServer.IsBound())
		{
			OnInitializeListenServer.Broadcast(NAME_GameSession);
		}
	}

	// Setup GameState variables if in GameplayLevel or if DedicatedServer
	if (bIsGameplayLevel || IsServer())
	{
		// setup existing players
		for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			PlayerTeamSetup(Iterator->Get());
		}
	}

	Super::BeginPlay();
}

APlayerController* AAccelByteWarsGameMode::Login(
	UPlayer* NewPlayer,
	ENetRole InRemoteRole,
	const FString& Portal,
	const FString& Options,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	APlayerController* PlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	// setup player if in GameplayLevel and game started
	if (bIsGameplayLevel || IsServer())
	{
		PlayerTeamSetup(PlayerController);
	}

	return PlayerController;
}

void AAccelByteWarsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (const AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(NewPlayer->PlayerState))
	{
		if (PlayerState->bShouldKick)
		{
			GameSession->KickPlayer(NewPlayer, FText::FromString("Max player reached"));
			GAMEMODE_LOG(Warning, TEXT("Player did not registered in Teams data. Max registered players reached. Kicking this player"));
			return;
		}
	}
}

void AAccelByteWarsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (IsServer())
	{
		if (bShouldRemovePlayerOnLogoutImmediately && !ABGameState->bIsServerTravelling)
		{
			const bool bSucceeded = RemovePlayer(Cast<APlayerController>(Exiting));
			GAMEMODE_LOG(Warning, TEXT("Removing player from GameState data. Succeeded: %s"), *FString(bSucceeded ? "TRUE" : "FALSE"));
		}
	}
}

APawn* AAccelByteWarsGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	// do not auto spawn pawn. We have our own handler.
	return nullptr;
}

void AAccelByteWarsGameMode::ResetGameData()
{
	ABGameState->EmptyTeams();
}

void AAccelByteWarsGameMode::PlayerTeamSetup(APlayerController* PlayerController) const
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerController null. Cancelling operation"));
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: PlayerState is not (derived from) AAccelByteWarsPlayerState. Cancelling operation"));
		return;
	}

	int32 TeamId = INDEX_NONE;
	const FUniqueNetIdRepl PlayerUniqueId = PlayerState->GetUniqueId();
	const int32 ControllerId = GetControllerId(PlayerState);
	
	// check for a match in GameState's Teams data
	if (FGameplayPlayerData* PlayerData =
		ABGameState->GetPlayerDataById(PlayerUniqueId, ControllerId))
	{
		// found
		// stored data doesn't have UniqueNetId while the given data has. Overwrite stored data
		if (PlayerState->GetUniqueId().IsValid() && !PlayerData->UniqueNetId.IsValid())
		{
			PlayerData->UniqueNetId = PlayerUniqueId;

			// notify local
			ABGameState->OnNotify_Teams();
		}

		// restore data
		TeamId = PlayerData->TeamId;
		PlayerState->SetPlayerName(PlayerData->PlayerName);
		PlayerState->AvatarURL = PlayerData->AvatarURL;
		PlayerState->SetScore(PlayerData->Score);
		PlayerState->TeamId = TeamId;
		PlayerState->NumLivesLeft = PlayerData->NumLivesLeft;
		PlayerState->KillCount = PlayerData->KillCount;
		PlayerState->TeamColor = GameInstance->GetTeamColor(TeamId);

#if UE_BUILD_DEVELOPMENT
		const bool bUniqueIdValid = PlayerUniqueId.GetUniqueNetId().IsValid();
		const FString Identity = bUniqueIdValid ?
			PlayerUniqueId.GetUniqueNetId()->ToDebugString() : PlayerController->PlayerState->GetPlayerName();
		GAMEMODE_LOG(
			Warning,
			TEXT("Found player's (%s [UniqueId: %s]) data in existing PlayerDatas. Assigning team: %d"),
			*Identity,
			*FString(bUniqueIdValid ? "TRUE" : "FALSE"),
			TeamId);
#endif
	}
	// flag to kick player if player's data was not found and max players reached (based on registered players)
	else
	{
		if (ABGameState->GetRegisteredPlayersNum() >= ABGameState->GameSetup.MaxPlayers)
		{
			// kick can happen as early as PostLogin
			PlayerState->bShouldKick = true;
			return;
		}
	}

	// if no match found, assign player to a new team or least populated team
	if (TeamId == INDEX_NONE)
	{
		if (IsServer())
		{
			// Running online, assign team from session info
			if (OnGetTeamIdFromSessionDelegate.IsBound()) 
			{
				OnGetTeamIdFromSessionDelegate.Broadcast(GameSession->SessionName, PlayerUniqueId, TeamId);
			}
			// Running offline, set team assignment manually
			else 
			{
				GAMEMODE_LOG(Warning, TEXT("PlayerTeamSetup: Delegate to get team id from game session is not bound. Cannot retrieve session info, reverting to offline DS flow"));

				AssignTeamManually(TeamId);
			}
		}
		// If running locally, set team assignment manually
		else 
		{
			AssignTeamManually(TeamId);
		}

		// reset player's state data
		PlayerState->SetPlayerName(TEXT(""));
		PlayerState->AvatarURL = TEXT("");
		PlayerState->TeamId = TeamId;
		PlayerState->TeamColor = GameInstance->GetTeamColor(TeamId);
		PlayerState->SetScore(0.0f);
        PlayerState->NumLivesLeft = INDEX_NONE;
        PlayerState->KillCount = 0;

#if UE_BUILD_DEVELOPMENT
		const bool bUniqueIdValid = PlayerUniqueId.GetUniqueNetId().IsValid();
		const FString Identity = bUniqueIdValid ?
			PlayerUniqueId.GetUniqueNetId()->ToDebugString() : PlayerController->PlayerState->GetPlayerName();
		GAMEMODE_LOG(
			Warning,
			TEXT("No player's (%s [UniqueId: %s]) data found. Assigning team: %d"),
			*Identity,
			*FString(bUniqueIdValid ? "TRUE" : "FALSE"),
			TeamId);
#endif
	}

	// Kick if the player doesn't belong to any team.
	PlayerState->bShouldKick = (TeamId == INDEX_NONE);
	if (PlayerState->bShouldKick) return;

	ABGameState->AddPlayerToTeam(
		TeamId,
		PlayerUniqueId,
		PlayerState->NumLivesLeft,
		ControllerId,
		PlayerState->GetScore(),
		PlayerState->KillCount);

	// If running online, refresh player's data based on AccelByte's data.
	if (PlayerController->GetNetMode() != ENetMode::NM_Standalone && OnAddOnlineMemberDelegate.IsBound())
	{
		OnAddOnlineMemberDelegate.Broadcast(PlayerController, TDelegate<void(bool)>::CreateWeakLambda(this, [this, PlayerController](bool bIsSuccessful)
		{
			UpdatePlayerInformation(PlayerController);
		}));
	}
}

void AAccelByteWarsGameMode::ServerTravel(TSoftObjectPtr<UWorld> Level)
{
	const FString Url = Level.GetLongPackageName();
	DelayedServerTravel(Url);
}

void AAccelByteWarsGameMode::DelayedServerTravel(const FString& URL) const
{
	if (ABGameState->bIsServerTravelling)
	{
		// if true, server already travelling, cancel operation
		return;
	}

	ABGameState->bIsServerTravelling = true;
	ABGameState->OnNotify_IsServerTravelling();

	// Delay server travel to let the game clients informed that the game is about to start.
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, URL]()
	{
		GetWorld()->ServerTravel(URL);
	}, 1.0f, false);
}

void AAccelByteWarsGameMode::AddPlayerToTeam(APlayerController* PlayerController, const int32 TeamId)
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("AddPlayerToTeam: PlayerController null. Cancelling operation"));
		return;
	}

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("AddPlayerToTeam: PlayerState is not (derived from) AAccelByteWarsPlayerState. Cancelling operation"));
		return;
	}

	const FUniqueNetIdRepl PlayerUniqueId = PlayerState->GetUniqueId();
	const int32 ControllerId = GetControllerId(PlayerState);

	// reset player's state data
	PlayerState->TeamId = TeamId;
	PlayerState->TeamColor = GameInstance->GetTeamColor(TeamId);
	PlayerState->SetScore(0.0f);
	PlayerState->NumLivesLeft = INDEX_NONE;
	PlayerState->KillCount = 0;

	// add player to team
	ABGameState->AddPlayerToTeam(
		TeamId,
		PlayerUniqueId,
		PlayerState->NumLivesLeft,
		ControllerId,
		PlayerState->GetScore(),
		PlayerState->KillCount);
}

bool AAccelByteWarsGameMode::RemovePlayer(const APlayerController* PlayerController) const
{
	// failsafe
	if (!PlayerController)
	{
		GAMEMODE_LOG(Warning, TEXT("RemovePlayer: PlayerController null. Cancelling operation"));
		return false;
	}

	const AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState)
	{
		GAMEMODE_LOG(Warning, TEXT("RemovePlayer: PlayerState is not (derived from) AAccelByteWarsPlayerState. Cancelling operation"));
		return false;
	}

	const FUniqueNetIdRepl PlayerUniqueId = PlayerState->GetUniqueId();
	const int32 ControllerId = GetControllerId(PlayerState);

	return ABGameState->RemovePlayerFromTeam(PlayerUniqueId, ControllerId);
}

void AAccelByteWarsGameMode::AssignTeamManually(int32& InOutTeamId) const
{
	switch (ABGameState->GameSetup.GameModeType)
	{
	case EGameModeType::FFA:
		// Try to assign to an empty team first.
		if (ABGameState->Teams.Num() >= ABGameState->GameSetup.MaxTeamNum)
		{
			// Assign to empty team.
			InOutTeamId = INDEX_NONE;
			for (const FGameplayTeamData& Team : ABGameState->Teams)
			{
				if (Team.TeamMembers.IsEmpty())
				{
					InOutTeamId = Team.TeamId;
					break;
				}
			}
		}
		// Assign to a new team
		else
		{
			InOutTeamId = ABGameState->Teams.Num();
		}
		break;
	case EGameModeType::TDM:
		// Try to assign to least populated team.
		if (ABGameState->Teams.Num() >= ABGameState->GameSetup.MaxTeamNum)
		{
			// The least populated team should less than the maximum players per team.
			uint8 CurrentTeamMemberNum = ABGameState->GameSetup.MaxPlayers / ABGameState->GameSetup.MaxTeamNum;
			InOutTeamId = INDEX_NONE;
			for (const FGameplayTeamData& Team : ABGameState->Teams)
			{
				if (Team.TeamMembers.Num() < CurrentTeamMemberNum)
				{
					CurrentTeamMemberNum = Team.TeamMembers.Num();
					InOutTeamId = Team.TeamId;
				}
			}
		}
		// Assign to a new team
		else
		{
			InOutTeamId = ABGameState->Teams.Num();
		}
		break;
	default:
		break;
	}
}

void AAccelByteWarsGameMode::UpdatePlayerInformation(const APlayerController* PlayerController) const
{
	if (const AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState))
	{
		FGameplayPlayerData* PlayerData =
			ABGameState->GetPlayerDataById(PlayerState->GetUniqueId(), GetControllerId(PlayerState));
		if (!PlayerData)
		{
			return;
		}

		if (!PlayerState->GetPlayerName().IsEmpty())
		{
			PlayerData->PlayerName = PlayerState->GetPlayerName();
		}
		PlayerData->AvatarURL = PlayerState->AvatarURL;

		// notify local
		ABGameState->OnNotify_Teams();
	}
}

int32 AAccelByteWarsGameMode::GetControllerId(const APlayerState* PlayerState)
{
	int32 ControllerId = 0;
	if (const APlayerController* PC = PlayerState->GetPlayerController())
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			ControllerId = LP->GetLocalPlayerIndex();
		}
	}
	return ControllerId;
}

bool AAccelByteWarsGameMode::IsServer() const
{
	const ENetMode NetMode = GetNetMode();
	return NetMode == ENetMode::NM_DedicatedServer || NetMode == ENetMode::NM_ListenServer;
}
