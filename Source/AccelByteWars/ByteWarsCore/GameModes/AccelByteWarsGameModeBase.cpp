// Copyright Epic Games, Inc. All Rights Reserved.


#include "AccelByteWarsGameModeBase.h"

#include "AccelByteWarsGameStateBase.h"
#include "ByteWarsCore/Player/AccelByteWarsPlayerState.h"
#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"
#include "ByteWarsCore/System/AccelByteWarsGlobals.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerState.h"

#define GAMEMODE_LOG(FormatString, ...) UE_LOG(LogByteWarsGameMode, Log, TEXT(FormatString), __VA_ARGS__);

void AAccelByteWarsGameModeBase::InitGameState()
{
	Super::InitGameState();

	AccelByteWarsGameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	AccelByteWarsGameState = GetGameState<AAccelByteWarsGameStateBase>();

	if (!ensure(AccelByteWarsGameInstance)) return;
	if (!ensure(AccelByteWarsGameState)) return;
}

void AAccelByteWarsGameModeBase::BeginPlay()
{
	// Check if GameSetup have already been set up or not
	if (!AccelByteWarsGameInstance->GameSetup)
	{
		// have not yet set up, set GameSetup based on launch argument
		FString CodeName;
		FParse::Value(FCommandLine::Get(), TEXT("-GameMode="), CodeName);
		AssignGameMode(CodeName);
		AccelByteWarsGameInstance->GameSetup = AccelByteWarsGameState->GameSetup;
	}

	// Setup GameState variables if in GameplayLevel
	if (bIsGameplayLevel)
	{
		// GameState setup
		AccelByteWarsGameState->GameSetup = AccelByteWarsGameInstance->GameSetup;
		const FGameModeData& GameModeData = AccelByteWarsGameState->GameSetup->SelectedGameMode.SelectedGameMode;
		AccelByteWarsGameState->TimeLeft = GameModeData.MatchTime;

		// setup existing players
		for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			PlayerSetup(Iterator->Get());
		}
	}

	Super::BeginPlay();
}

APlayerController* AAccelByteWarsGameModeBase::Login(
	UPlayer* NewPlayer,
	ENetRole InRemoteRole,
	const FString& Portal,
	const FString& Options,
	const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	APlayerController* PlayerController = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	// setup player if in GameplayLevel and game started
	if (bIsGameplayLevel && HasMatchStarted())
	{
		PlayerSetup(PlayerController);
	}

	return PlayerController;
}

int32 AAccelByteWarsGameModeBase::AddPlayerScore(APlayerState* PlayerState, const float InScore, const bool bAddKillCount)
{
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AccelByteWarsPlayerState)
	{
		GAMEMODE_LOG("PlayerState is not derived from AAccelByteWarsPlayerState. Operation cancelled")
		return INDEX_NONE;
	}

	FGameplayPlayerData* PlayerData =
		AccelByteWarsGameState->PlayerDatas.FindByKey(PlayerState->GetUniqueId());
	if (!PlayerData)
	{
		GAMEMODE_LOG("Player is not in PlayerDatas. Add player to team via AddToTeam. Operation cancelled")
		return INDEX_NONE;
	}

	// set score in PlayerState
	const float FinalScore = AccelByteWarsPlayerState->GetScore() + InScore;
	AccelByteWarsPlayerState->SetScore(FinalScore);

	// set score in GameState
	PlayerData->Score = FinalScore;

	// increase kill count
	if (bAddKillCount)
	{
		AccelByteWarsPlayerState->KillCount++;
		PlayerData->KillCount = AccelByteWarsPlayerState->KillCount;
	}

	return AccelByteWarsPlayerState->GetScore();
}

int32 AAccelByteWarsGameModeBase::DecreasePlayerLife(APlayerState* PlayerState, const uint8 Decrement)
{
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerState);
	if (!AccelByteWarsPlayerState)
	{
		GAMEMODE_LOG("PlayerState is not derived from AAccelByteWarsPlayerState. Operation cancelled")
		return INDEX_NONE;
	}

	FGameplayPlayerData* PlayerData = AccelByteWarsGameState->PlayerDatas.FindByKey(PlayerState->GetUniqueId());
	if (!PlayerData)
	{
		GAMEMODE_LOG("Player is not in PlayerDatas. Add player to team via AddToTeam. Operation cancelled")
		return INDEX_NONE;
	}

	// decrease life num in PlayerState
	AccelByteWarsPlayerState->NumLivesLeft -= Decrement;

	// match life num in GameState to PlayerState
	PlayerData->NumLivesLeft = AccelByteWarsPlayerState->NumLivesLeft;

	return AccelByteWarsPlayerState->NumLivesLeft;
}

#pragma region "GameSetup related"
void AAccelByteWarsGameModeBase::PlayerSetup(APlayerController* PlayerController) const
{
	// failsafe
	if (!PlayerController) return;

	AAccelByteWarsPlayerState* PlayerState = static_cast<AAccelByteWarsPlayerState*>(PlayerController->PlayerState);
	if (!PlayerState) return;

	int32 TeamId = INDEX_NONE;
	const FUniqueNetIdRepl PlayerUniqueId = GetPlayerUniqueNetId(PlayerController);

	// check for a match in GameState's PlayerDatas | restoring data for disconnected player
	bool bFoundInPlayerDatas = false;
	if (const FGameplayPlayerData* PlayerData = AccelByteWarsGameState->PlayerDatas.FindByKey(PlayerUniqueId))
	{
		// found, restore data
		bFoundInPlayerDatas = true;
		TeamId = PlayerData->TeamId;
		PlayerState->SetScore(PlayerData->Score);
		PlayerState->TeamId = TeamId;
		PlayerState->NumLivesLeft = PlayerData->NumLivesLeft;
		PlayerState->KillCount = PlayerData->KillCount;
		PlayerState->TeamColor = AccelByteWarsGameState->GetTeamColor(TeamId);

#if UE_BUILD_DEVELOPMENT
		const FString Identity = PlayerUniqueId.GetUniqueNetId().IsValid() ?
			PlayerUniqueId.GetUniqueNetId()->ToDebugString() : PlayerController->PlayerState->GetPlayerName();
		GAMEMODE_LOG("Found player's (%s) data in existing PlayerDatas. Assigning team: %d", *Identity, TeamId);
#endif
	}

	// check for a match in GameState's GameSetup
	bool bFoundInGameSetup = false;
	for (const UAccelByteWarsTeamSetup* TeamSetup : AccelByteWarsGameState->GameSetup->TeamSetups)
	{
		for (const UAccelByteWarsPlayerSetup* PlayerSetup : TeamSetup->PlayerSetups)
		{
			if (PlayerSetup->UniqueNetId == PlayerUniqueId)
			{
				// found restore data
				bFoundInGameSetup = true;
				TeamId = TeamSetup->TeamId;
				PlayerState->TeamId = TeamId;
				PlayerState->TeamColor = TeamSetup->TeamColour;

#if UE_BUILD_DEVELOPMENT
				const FString Identity = PlayerUniqueId.GetUniqueNetId().IsValid() ?
					PlayerUniqueId.GetUniqueNetId()->ToDebugString() : PlayerController->PlayerState->GetPlayerName();
				GAMEMODE_LOG("Found player's (%s) data in existing TeamSetup. Assigning team: %d", *Identity, TeamId);
#endif
				break;
			}
		}
		if (bFoundInGameSetup) break;
	}

	// kick player if player's data was not found and max players reached (based on registered players)
	if (!bFoundInGameSetup && !bFoundInPlayerDatas)
	{
		if (AccelByteWarsGameState->GameSetup->SelectedGameMode.RegisteredPlayerCount >= AccelByteWarsGameState->GameSetup->SelectedGameMode.SelectedGameMode.MaxPlayers)
		{
			GameSession->KickPlayer(PlayerController, FText::FromString("Max players reached"));
			return;
		}
	}

	// if no match found, assign player to a new team or least populated team
	if (TeamId == INDEX_NONE)
	{
		switch (AccelByteWarsGameState->GameSetup->SelectedGameMode.SelectedGameMode.GameModeType)
		{
		case EGameModeType::FFA:
			// assign to a new team
			TeamId = AccelByteWarsGameState->GameSetup->TeamSetups.Num();
			break;
		case EGameModeType::TDM:
			// check if max team reached
			if (AccelByteWarsGameState->GameSetup->TeamSetups.Num() >= AccelByteWarsGameState->GameSetup->SelectedGameMode.SelectedGameMode.MaxTeamNum)
			{
				// assign to the least populated team
				uint8 CurrentTeamMemberNum = UINT8_MAX;
				TeamId = 0;
				for (const UAccelByteWarsTeamSetup* TeamSetup : AccelByteWarsGameState->GameSetup->TeamSetups)
				{
					if (TeamSetup->PlayerSetups.Num() < CurrentTeamMemberNum)
					{
						CurrentTeamMemberNum = TeamSetup->PlayerSetups.Num();
						TeamId = TeamSetup->TeamId;
					}
				}
			}
			else
			{
				// assign to a new team
				TeamId = AccelByteWarsGameState->GameSetup->TeamSetups.Num();
			}
			break;
		default: ;
		}

		// set player's state data
		PlayerState->TeamId = TeamId;
		PlayerState->TeamColor = AccelByteWarsGameState->GetTeamColor(TeamId);

#if UE_BUILD_DEVELOPMENT
		const FString Identity = PlayerUniqueId.GetUniqueNetId().IsValid() ?
			PlayerUniqueId.GetUniqueNetId()->ToDebugString() : PlayerController->PlayerState->GetPlayerName();
		GAMEMODE_LOG("No player's (%s) data found. Assigning team: %d", *Identity, TeamId);
#endif
	}

	// add player to team
	AccelByteWarsGameState->AddPlayerToTeam(
		TeamId,
		PlayerUniqueId,
		FName(PlayerController->PlayerState->GetPlayerName()),
		PlayerState->GetScore(),
		PlayerState->NumLivesLeft,
		PlayerState->KillCount,
		!bFoundInGameSetup,
		!bFoundInPlayerDatas
	);

	// update team-related PlayerState
	AAccelByteWarsPlayerState* AccelByteWarsPlayerState = Cast<AAccelByteWarsPlayerState>(PlayerController->PlayerState);
	if (!ensure(AccelByteWarsPlayerState)) return;
	AccelByteWarsPlayerState->TeamColor = AccelByteWarsGameState->GameSetup->TeamSetups[TeamId]->TeamColour;
	AccelByteWarsPlayerState->TeamId = TeamId;
}

void AAccelByteWarsGameModeBase::AssignGameMode(FString CodeName) const
{
	// reset GameData
	AccelByteWarsGameState->GameSetup = nullptr;
	AccelByteWarsGameState->PlayerDatas.Empty();

	const FGameModeData GameModeData = GetGameModeDataByCodeName(CodeName);

	// set GameSetup
	FSelectedGameMode SelectedGameMode = FSelectedGameMode{GameModeData};
	UAccelByteWarsGameSetup* GameSetup = NewObject<UAccelByteWarsGameSetup>();
	GameSetup->SelectedGameMode = SelectedGameMode;
	AccelByteWarsGameState->GameSetup = GameSetup;

	GAMEMODE_LOG("Game mode set to GameState: %s", *GameModeData.CodeName);
}

FGameModeData AAccelByteWarsGameModeBase::GetGameModeDataByCodeName(const FString CodeName) const
{
	FGameModeData Data;
	if (ensure(GameModeDataTable))
	{
		TArray<FGameModeData*> GameModeDatas;
		GameModeDataTable->GetAllRows("GetGameModeDataByCodeName", GameModeDatas);

		if (GameModeDatas.Num() > 0)
		{
			for (const FGameModeData* GameModeData : GameModeDatas)
			{
				if (GameModeData->CodeName.Equals(CodeName))
				{
					Data = *GameModeData;
					break;
				}
			}

			// fallback
			Data = *GameModeDatas[0];
		}
	}
	return Data;
}
#pragma endregion

FUniqueNetIdRepl AAccelByteWarsGameModeBase::GetPlayerUniqueNetId(const APlayerController* PlayerController)
{
	FUniqueNetIdRepl NetId;

	if (PlayerController->IsLocalController())
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			NetId = LocalPlayer->GetPreferredUniqueNetId();
		}
	}
	else
	{
		NetId = PlayerController->PlayerState->GetUniqueId();
	}

	return NetId;
}
