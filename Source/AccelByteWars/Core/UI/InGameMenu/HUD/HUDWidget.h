// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"
#include "Core/PowerUps/PowerUpModels.h"
#include "HUDWidget.generated.h"

class UHUDWidgetEntry;
class AAccelByteWarsInGameGameState;

UCLASS()
class ACCELBYTEWARS_API UHUDWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	/**
	 * @brief Set HUD entries value
	 * @param Value Value to be displayed
	 * @param Index Team's index. Currently, this HUD only supports 4 team (0 - 3)
	 * @param BoxIndex Box index. 0 = left; 1 = middle; 2 = right
	 */
	UFUNCTION(BlueprintCallable)
	bool SetValue(const FString Value, const int32 Index, const int32 BoxIndex);

	/**
	 * @brief Set HUD power up entries value
	 * @param SelectedPowerUps All power-ups equipped by team members.
	 * @param TeamIndex Team's index. Currently, this HUD only supports 4 team (0 - 3)
	 */
	UFUNCTION(BlueprintCallable)
	bool SetPowerUps(const TArray<TEnumAsByte<EPowerUpSelection>>& SelectedPowerUps, const TArray<int32>& PowerUpCounts, const int32 TeamIndex);

	/**
	 * @brief Change the color of HUD entry. Will only be executed when Color is not the same with current HUD entry's color
	 * @param Index Team's index. Currently, this HUD only supports 4 team (0 - 3)
	 * @param Color Team's color
	 */
	UFUNCTION(BlueprintCallable)
	bool SetColorChecked(const int32 Index, const FLinearColor Color);

	/**
	 * @brief Toggle activate or deactivate HUD entry.
	 * @param Index Team's index. Currently, this HUD only supports 4 team (0 - 3)
	 * @param bActivate Set it to true will activate the widget and vice versa.
	 */
	UFUNCTION(BlueprintCallable)
	bool ToggleEntry(const int32 Index, const bool bActivate);

	/**
	 * @brief Set Time Left value
	 * @param TimeLeft Time Left
	 */
	UFUNCTION(BlueprintCallable)
	void SetTimerValue(const float TimeLeft);

	/**
	 * @brief Retrieve the start and end position, on screen, of the top bar on the HUD
	 * @param OutMinPixelPosition Geometry start position
	 * @param OutMaxPixelPosition Geometry end position
	 */
	UFUNCTION(BlueprintCallable)
	void GetVisibleHUDPixelPosition(FVector2D& OutMinPixelPosition, FVector2D& OutMaxPixelPosition) const;

private:
	/* Used to tell the game what size the visible HUD is */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* Widget_VisibleBorder;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHUDWidgetEntry* Widget_HUDNameValueP1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHUDWidgetEntry* Widget_HUDNameValueP2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHUDWidgetEntry* Widget_HUDNameValueP3;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHUDWidgetEntry* Widget_HUDNameValueP4;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHUDWidgetEntry* Widget_HUDNameValueTimer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* Widget_PreGameCountdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* Widget_NotEnoughPlayerCountdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* Widget_SimulateServerCrashCountdown;

	UPROPERTY()
	AAccelByteWarsInGameGameState* ByteWarsGameState;

	UFUNCTION()
	ECountdownState SetPreGameCountdownState() const;

	UFUNCTION()
	int UpdatePreGameCountdownValue() const;

	UFUNCTION()
	void OnPreGameCountdownFinished();

	UFUNCTION()
	ECountdownState SetNotEnoughPlayerCountdownState() const;

	UFUNCTION()
	int UpdateNotEnoughPlayerCountdownValue() const;

	UFUNCTION()
	void OnNotEnoughPlayerCountdownFinished();

	UFUNCTION()
	ECountdownState SetSimulateServerCrashCountdownState() const;

	UFUNCTION()
	int UpdateSimulateServerCrashCountdownValue() const;

	UFUNCTION()
	void OnSimulateServerCrashCountdownFinished();

	FDelegateHandle OnPreGameCountdownFinishedDelegateHandle;
	FDelegateHandle OnNotEnoughPlayerCountdownFinishedDelegateHandle;
	FDelegateHandle OnSimulateServerCrashCountdownFinishedDelegateHandle;
};
