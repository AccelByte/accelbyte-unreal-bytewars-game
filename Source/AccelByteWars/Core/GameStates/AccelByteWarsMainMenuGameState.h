// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "AccelByteWarsGameState.h"
#include "AccelByteWarsMainMenuGameState.generated.h"

#pragma region "Structs, Enums, and Delegates declaration"
UENUM(BlueprintType)
enum class ELobbyStatus : uint8
{
	IDLE = 0,
	LOBBY_COUNTDOWN_STARTED,
	GAME_STARTED,
	NOT_ENOUGH_PLAYER,
};
#pragma endregion 

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsMainMenuGameState : public AAccelByteWarsGameState
{
	GENERATED_BODY()

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	float GetLobbyShutdownCountdown() const;
	void ReduceLobbyShutdownCountdown(const float DeltaSeconds);
	void ResetLobbyShutdownCountdown();

protected:
	virtual void BeginPlay() override;

public:
	// Current lobby status.
	UPROPERTY(BlueprintReadWrite, Replicated)
	ELobbyStatus LobbyStatus = ELobbyStatus::IDLE;
	
	// Lobby countdown before starting the game.
	UPROPERTY(BlueprintReadWrite, Replicated)
	float LobbyCountdown = 30.f;

private:
	// Lobby countdown before starting the game.
	UPROPERTY(Replicated)
	float LobbyShutdownCountdown = 30.f;
};
