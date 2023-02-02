// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/Settings/GlobalSettingsDataAsset.h"
#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "AccelByteWarsGameStateBase.generated.h"

DECLARE_LOG_CATEGORY_CLASS(LogByteWarsGameState, Log, All);

#pragma region "Structs and data storing purpose UObject declaration"
/**
 * @brief Data that needs to be persistent throughout the gameplay even when player logged out
 */
USTRUCT(BlueprintType)
struct FGameplayPlayerData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FUniqueNetIdRepl UniqueNetId;

	UPROPERTY(BlueprintReadWrite)
	int32 TeamId = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite)
	float Score = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 NumLivesLeft = 1;

	bool operator==(const FGameplayPlayerData& Other) const
	{
		return UniqueNetId == Other.UniqueNetId;
	}
	
	bool operator==(const FUniqueNetIdRepl& Other) const
	{
		return UniqueNetId == Other;
	}
};
#pragma endregion 

/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(BlueprintReadWrite)
	UAccelByteWarsGameSetup* GameSetup = nullptr;

	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnNotify_PlayerDatas, Replicated)
	TArray<FGameplayPlayerData> PlayerDatas;

	UFUNCTION(BlueprintImplementableEvent)
	void OnNotify_PlayerDatas();

	UPROPERTY(BlueprintReadWrite, Replicated)
	float TimeLeft = INDEX_NONE;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsGameOver = false;

	UFUNCTION(BlueprintCallable)
	EGameModeType GetCurrentGameModeType() const;

	UFUNCTION(BlueprintCallable)
	TArray<int32> GetRemainingTeams() const;

	UFUNCTION(BlueprintCallable)
	UAccelByteWarsTeamSetup* GetTeamInfoByTeamId(
		const int32 TeamId,
		float& OutTeamScore,
		int32& OutTeamLivesLeft,
		TArray<FGameplayPlayerData>& OutTeamMemberDatas);

	/**
	 * @brief Add player to GameSetup.TeamSetup and to PlayerDatas. Will add new team if team does not exist
	 * @param TeamId Target Team id to add to
	 * @param UniqueNetId Player's unique net id
	 * @param PlayerName Player's name
	 * @param Score Specify player's score
	 * @param Lives Specify player's lives num | set -1 to use GameState.GameData.GameSetup's value
	 * @param KillCount Specify player's kill count
	 * @param bAddToGameSetup Whether to add this data to GameData.GameSetup.TeamSetup.PlayerSetup
	 * @param bAddToPlayerData Whether to add this data to GameData.PlayerDatas
	 * @return true if operation successful
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool AddPlayerToTeam(
		uint8 TeamId,
		FUniqueNetIdRepl UniqueNetId,
		FName PlayerName,
		const float Score = 0.0f,
		const int32 Lives = -1,
		const int32 KillCount = 0,
		const bool bAddToGameSetup = true,
		const bool bAddToPlayerData = true);

	/**
	 * @brief Remove Player from a team
	 * @param UniqueNetId Player's unique net id
	 * @return true if operation successful
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool RemovePlayerFromTeam(FUniqueNetIdRepl UniqueNetId);

	/**
	 * @brief Remove Player from a team, more efficient than RemovePlayerFromTeam
	 * @param TeamId TeamId to remove player from
	 * @param UniqueNetId Player's unique net id
	 * @return true if operation successful
	 */
	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool RemovePlayerFromTeamByTeamId(int32 TeamId, FUniqueNetIdRepl UniqueNetId);

	UFUNCTION(BlueprintCallable)
	FLinearColor GetTeamColor(uint8 TeamId) const;

private:

	UPROPERTY(EditAnywhere)
	UGlobalSettingsDataAsset* GlobalSettingsDataAsset;
};
