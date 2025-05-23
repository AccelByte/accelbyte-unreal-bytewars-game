// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/UI/InGameMenu/HUD/HUDPlayer.h"
#include "Net/UnrealNetwork.h"
#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerController.h"
#include "AccelByteWarsPlayerController.generated.h"

UCLASS()
class ACCELBYTEWARS_API AAccelByteWarsPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
	void DelayedClientTravel(TSoftObjectPtr<UWorld> Level);

	void DelayedClientTravel(const FString& Url, const ETravelType TravelType);
	
	// Trigger to start the game from game lobby.
	UFUNCTION(Reliable, Server, meta = (WorldContext = "WorldContextObject"))
	void TriggerLobbyStart();

	void LoadingPlayerAssignment() const;

	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());

	/**
	* @brief Pointer to custom HUD
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AccelByteWars)
	AHUDPlayer* ABPlayerHUD = nullptr;

	/**
	 * @brief Generic OnRep notify for player ship design
	 */
	UFUNCTION()
	void OnRepNotify_ShipDesign();

private:
	bool bDelayedClientTravelStarted = false;

#pragma region "Testing purposes"
public:
	UFUNCTION()
	void ClientInstructInstaKillPlayer(const TArray<APlayerState*>& PlayerStates);

protected:
	UFUNCTION(Server, Reliable)
	void ServerInstructInstaKillPlayer(const TArray<APlayerState*>& PlayerStates);
#pragma endregion 
};