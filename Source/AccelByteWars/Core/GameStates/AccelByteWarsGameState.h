// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "AccelByteWarsGameState.generated.h"

ACCELBYTEWARS_API DECLARE_LOG_CATEGORY_EXTERN(LogAccelByteWarsGameState, Log, All);
#define GAMESTATE_LOG(Verbosity, Format, ...) \
{ \
	UE_LOG(LogAccelByteWarsGameState, Verbosity, TEXT("%s"), *FString::Printf(Format, ##__VA_ARGS__)); \
}

#pragma region "Structs, Enums, and Delegates declaration"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGameStateVoidDelegate);
#pragma endregion 

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	//~AActor overriden functions
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	//~End of AActor overriden functions

	/**
	 * @brief Game setup info
	 */
	UPROPERTY(BlueprintReadOnly, Replicated)
	FGameModeData GameSetup;

	/**
	 * @brief Teams info and data
	 */
	UPROPERTY(BlueprintReadWrite, Replicated, ReplicatedUsing = OnNotify_Teams)
	TArray<FGameplayTeamData> Teams;

	UPROPERTY(Replicated, ReplicatedUsing = OnNotify_IsServerTravelling)
	bool bIsServerTravelling = false;

	UPROPERTY(BlueprintAssignable)
	FGameStateVoidDelegate OnIsServerTravellingChanged;

	UPROPERTY(BlueprintAssignable)
	FGameStateVoidDelegate OnTeamsChanged;

	// Static delegate to be called when the game state is initialized and replicated.
	inline static FSimpleMulticastDelegate OnInitialized;

	UFUNCTION()
	void OnNotify_IsServerTravelling() const;

	UFUNCTION()
	void OnNotify_Teams();

	UFUNCTION(BlueprintCallable)
	void EmptyTeams();

	UFUNCTION(BlueprintCallable)
	void AssignGameMode(const FString& CodeName);

	void AssignCustomGameMode(const FOnlineSessionSettings* Setting);

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
	 * @brief Add player to GameSetup.TeamSetup. Will add new team if team does not exist
	 * @param TeamId Target Team id to add to
	 * @param UniqueNetId Player's unique net id
	 * @param Score Specify player's score
	 * @param OutLives Specify player's lives num | set -1 to use GameState.GameData.GameSetup's value | will change the referenced value
	 * @param ControllerId Player's controller id
	 * @param KillCount Specify player's kill count
	 * @param PlayerName Specify player's name
	 * @param AvatarURL Specify player's avatar's picture URL
	 * @param bForce Force add player even if the player data already exist
	 * @return true if operation successful
	 */
	bool AddPlayerToTeam(
		int8 TeamId,
		FUniqueNetIdRepl UniqueNetId,
		UPARAM(ref) int32& OutLives,
		const int32 ControllerId = 0,
		const float Score = 0.0f,
		const int32 KillCount = 0,
		const FString PlayerName = TEXT(""),
		const FString AvatarURL = TEXT(""),
		const bool bForce = false);

	/**
	 * @brief Remove Player from a team
	 * @param UniqueNetId Player's unique net id
	 * @param ControllerId Player's Local ControllerId
	 * @return true if operation successful
	 */
	bool RemovePlayerFromTeam(FUniqueNetIdRepl UniqueNetId, const int32 ControllerId = 0);
	
	// Countdown to show that the server is simulated to crash.
	UPROPERTY(Replicated)
	float SimulateServerCrashCountdown = INDEX_NONE;

private:

	UPROPERTY()
	UAccelByteWarsGameInstance* GameInstance = nullptr;

	/**
	 * @brief If true, store Teams and GameSetup to GameInstance before travel
	 */
	UPROPERTY(EditAnywhere)
	bool bAutoBackupData = true;

	/**
	 * @brief If true, restore Teams and GameSetup from GameInstance on PostInitializeComponent
	 */
	UPROPERTY(EditAnywhere)
	bool bAutoRestoreData = true;
};
