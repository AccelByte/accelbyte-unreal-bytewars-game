// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "AccelByteWars/Core/UI/InGameMenu/HUD/HUDPlayer.h"
#include "Core/PowerUps/PowerUpModels.h"
#include "Core/Ships/PlayerShipModels.h"
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

	void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
	void DelayedClientTravel(TSoftObjectPtr<UWorld> Level);

	void DelayedClientTravel(const FString& Url, const ETravelType TravelType);
	
	// Trigger to start the game from game lobby.
	UFUNCTION(Reliable, Server, meta = (WorldContext = "WorldContextObject"))
	void TriggerLobbyStart();

	void LoadingPlayerAssignment() const;

	TSharedRef<FOnlineSessionSearch> SessionSearch = MakeShared<FOnlineSessionSearch>(FOnlineSessionSearch());

	/**
	 * @brief Sets the currently selected ship design
	 */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = AccelByteWars)
	void Server_SelectPlayerShipDesign(const EShipDesign SelectedShipDesign);

	/**
	* @brief Enum that indicates the ship design the player selected in the main menu
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AccelByteWars, ReplicatedUsing = OnRepNotify_ShipDesign)
	TEnumAsByte<EShipDesign> ShipDesign = EShipDesign::TRIANGLE;

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
};