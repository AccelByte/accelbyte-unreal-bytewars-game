// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "AccelByteWarsMainMenuGameMode.generated.h"

#pragma region "UI Text macros"
#define TEXT_CONNECTION_FAILED NSLOCTEXT("AccelByteWars", "connection_failed", "Connection Failed")
#define TEXT_CONNECTION_FAILED_GENERIC NSLOCTEXT("AccelByteWars", "connection_failed", "Failed to connect to server.")

#define TEXT_ERROR_NET_DRIVER_EXIST NSLOCTEXT("AccelByteWars", "error_net_driver_exit", "Network driver already exist.")
#define TEXT_ERROR_NET_DRIVER_INIT NSLOCTEXT("AccelByteWars", "error_net_driver_init", "Error creating network driver.")
#define TEXT_ERROR_NET_DRIVER_LISTEN NSLOCTEXT("AccelByteWars", "error_net_driver_init", "Network driver failed to listen.")
#define TEXT_CONNECTION_LOST NSLOCTEXT("AccelByteWars", "connection_lost", "Your connection to the host has been lost.")
#define TEXT_CONNECTION_TIMEOUT NSLOCTEXT("AccelByteWars", "connection_timeout", "Your connection to the host has been terminated. Took too long to respond.")
#define TEXT_CLIENT_OUTDATED NSLOCTEXT("AccelByteWars", "client_outdated", "The match you are trying to join is running an incompatible version of the game.")
#define TEXT_SERVER_OUTDATED NSLOCTEXT("AccelByteWars", "server_outdated", "Client trying to connect, running an incompatible version of the game")
#define TEXT_PENDING_CONNECTION_FAILED NSLOCTEXT("AccelByteWars", "pending_connection_failed", "Pending connection failed.")
#pragma endregion 

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsMainMenuGameMode : public AAccelByteWarsGameMode
{
	GENERATED_BODY()

public:
	virtual void InitGameState() override;
	virtual void BeginPlay() override;
	virtual APlayerController* Login(
		UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void CreateLocalGameSetup(const FString& CodeName, const int32 LocalPlayerNum);

private:
	UPROPERTY()
	AAccelByteWarsMainMenuGameState* ABMainMenuGameState = nullptr;
};
