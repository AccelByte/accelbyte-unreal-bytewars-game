// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "AccelByteWarsGameStateBase.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsGameState, Log, All);

#define GAMESTATE_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsGameState, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

UENUM(BlueprintType)
enum class EGameStatus : uint8
{
	IDLE = 0,
	AWAITING_PLAYERS_DS,
	AWAITING_PLAYERS,
	PRE_GAME_COUNTDOWN_STARTED,
	GAME_STARTED,
	AWAITING_PLAYERS_MID_GAME,
	GAME_ENDS,
	INVALID
};

UENUM(BlueprintType)
enum class ELobbyStatus : uint8
{
	IDLE = 0,
	LOBBY_COUNTDOWN_STARTED,
	GAME_STARTED
};

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

	//~AActor overriden functions
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	//~End of AActor overriden functions

public:
	UPROPERTY(BlueprintReadWrite, Replicated)
	EGameModeType GameModeType = EGameModeType::FFA;

	/**
	 * @brief Teams info and data
	 */
	UPROPERTY(BlueprintReadWrite, Replicated, ReplicatedUsing = OnNotify_Teams)
	TArray<FGameplayTeamData> Teams;

	UFUNCTION(BlueprintImplementableEvent)
	void OnNotify_Teams();

	// Current lobby status.
	UPROPERTY(BlueprintReadWrite, Replicated)
	ELobbyStatus LobbyStatus = ELobbyStatus::IDLE;

	// Lobby countdown before starting the game.
	UPROPERTY(BlueprintReadWrite, Replicated)
	float LobbyCountdown = 30.f;

	// Countdown when the game is over
	UPROPERTY(Replicated)
	float PostGameCountdown = INDEX_NONE;

	UPROPERTY(Replicated)
	float NotEnoughPlayerCountdown = INDEX_NONE;

	/**
	 * @brief Pre-game (already in gameplay map) countdown duration
	 */
	UPROPERTY(BlueprintReadWrite, Replicated)
	float PreGameCountdown = 5.0f;

	UPROPERTY(BlueprintReadWrite, Replicated)
	float TimeLeft = INDEX_NONE;

	/**
	 * @brief Current gameplay state
	 */
	UPROPERTY(BlueprintReadWrite, Replicated)
	EGameStatus GameStatus = EGameStatus::IDLE;

	/**
	 * @brief Get teams with at least one member have life count more than 1
	 * @return TeamIds
	 */
	UFUNCTION(BlueprintCallable)
	TArray<int32> GetRemainingTeams() const;

	/**
	 * @brief Get Team info and data by Team Id
	 * @param TeamId Target TeamId
	 * @param OutTeamData Output: Team info and data
	 * @param OutTeamScore Output: Total team score
	 * @param OutTeamLivesLeft Output: Total team lives count
	 * @param OutTeamKillCount Output: Total team kill count
	 * @return True if team found	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool GetTeamDataByTeamId(
		const int32 TeamId,
		FGameplayTeamData& OutTeamData,
		float& OutTeamScore,
		int32& OutTeamLivesLeft,
		int32& OutTeamKillCount);

	/**
	 * @brief Get player's data by unique net id or controller id
	 * @param UniqueNetId Target player's unique net id
	 * @param OutPlayerData Output: Player data
	 * @param ControllerId Target player's controller id | will only be used if unique net id is not valid
	 * @return True if player found
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool GetPlayerDataById(
		const FUniqueNetIdRepl UniqueNetId,
		FGameplayPlayerData& OutPlayerData,
		const int32 ControllerId = 0);

	/**
	 * @brief Get player's data by unique net id or controller id
	 * @param UniqueNetId Target player's unique net id
	 * @param ControllerId Target player's controller id | will only be used if unique net id is not valid
	 * @return Player data | nullptr if not found
	 */
	FGameplayPlayerData* GetPlayerDataById(const FUniqueNetIdRepl UniqueNetId, const int32 ControllerId = 0);

	/**
	 * @brief Get player count registered in GameState's game data
	 * @return Registered players count
	 */
	UFUNCTION(BlueprintCallable)
	int32 GetRegisteredPlayersNum() const;

	/**
	 * @brief Get player's default username (e.g. Player 1, Player 2, etc.)
	 * @return Default player username.
	*/
	UFUNCTION(BlueprintCallable)
	FString GetPlayerDefaultUsername(const FUniqueNetIdRepl UniqueNetId, const int32 ControllerId);

	/**
	 * @brief Add player to GameSetup.TeamSetup and to PlayerDatas. Will add new team if team does not exist
	 * @param TeamId Target Team id to add to
	 * @param UniqueNetId Player's unique net id
	 * @param Score Specify player's score
	 * @param OutLives Specify player's lives num | set -1 to use GameState.GameData.GameSetup's value | will change the referenced value
	 * @param ControllerId Player's controller id
	 * @param KillCount Specify player's kill count
	 * @param bForce Force add player even if the player data already exist
	 * @return true if operation successful
	 */
	bool AddPlayerToTeam(
		uint8 TeamId,
		FUniqueNetIdRepl UniqueNetId,
		UPARAM(ref) int32& OutLives,
		const int32 ControllerId = 0,
		const float Score = 0.0f,
		const int32 KillCount = 0, const bool bForce = false);

	/**
	 * @brief Remove Player from a team
	 * @param UniqueNetId Player's unique net id
	 * @return true if operation successful
	 */
	bool RemovePlayerFromTeam(FUniqueNetIdRepl UniqueNetId, const int32 ControllerId = 0);

private:

	UPROPERTY()
	UAccelByteWarsGameInstance* ByteWarsGameInstance = nullptr;

	UPROPERTY(EditAnywhere)
	bool bAutoBackupTeamsData = true;

	UPROPERTY(EditAnywhere)
	bool bAutoRestoreTeamsData = true;
};
