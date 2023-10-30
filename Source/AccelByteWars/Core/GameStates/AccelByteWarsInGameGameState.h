// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsGameState.h"
#include "AccelByteWarsInGameGameState.generated.h"

class UAccelByteWarsGameplayObjectComponent;

#pragma region "Structs, Enums, and Delegates declaration"
UENUM(BlueprintType)
enum class EGameStatus : uint8
{
	IDLE = 0,
	AWAITING_PLAYERS,
	PRE_GAME_COUNTDOWN_STARTED,
	GAME_STARTED,
	AWAITING_PLAYERS_MID_GAME,
	GAME_ENDS_DELAY,
	GAME_ENDS,
	INVALID
};
#pragma endregion 

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsInGameGameState : public AAccelByteWarsGameState
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable)
	bool HasGameStarted() const;

	UFUNCTION(BlueprintCallable)
	bool HasGameEnded() const;

	/**
	 * @brief Current gameplay state
	 */
	UPROPERTY(BlueprintReadWrite, Replicated)
	EGameStatus GameStatus = EGameStatus::IDLE;

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

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector2D MinGameBound = {-1000.0, -900.0};

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector2D MaxGameBound = { 1000.0, 900.0 };

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector2D MinStarsGameBound = {-1500.0, -1300.0};

	UPROPERTY(BlueprintReadWrite, Replicated)
	FVector2D MaxStarsGameBound = {1500.0, 1300.0};

	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<UAccelByteWarsGameplayObjectComponent*> ActiveGameObjects;
};
