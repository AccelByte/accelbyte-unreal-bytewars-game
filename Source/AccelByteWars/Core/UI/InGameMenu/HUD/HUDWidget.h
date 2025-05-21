// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"
#include "HUDWidget.generated.h"

class UHorizontalBox;
class UHUDWidgetEntry;
class UPushNotificationWidget;
class AAccelByteWarsInGameGameState;

UCLASS()
class ACCELBYTEWARS_API UHUDWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	/**
	 * @brief Generate HUD widget entries based on the team ids.
	 */
	UFUNCTION(BlueprintCallable)
	void GenerateHUDEntries();

	/**
	 * @brief Update HUD to the current data of teams and player states
	 */
	UFUNCTION(BlueprintCallable)
	void UpdateHUDEntries();

	/**
	 * @brief Retrieve the start and end position, on screen, of the top bar on the HUD
	 * @param OutMinPixelPosition Geometry start position
	 * @param OutMaxPixelPosition Geometry end position
	 */
	UFUNCTION(BlueprintCallable)
	void GetVisibleHUDPixelPosition(FVector2D& OutMinPixelPosition, FVector2D& OutMaxPixelPosition) const;

private:
	UFUNCTION(BlueprintCallable)
	void SetTimerValue(const float TimeLeft);

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

	UFUNCTION()
	void OnPlayerDie(const AAccelByteWarsPlayerState* DeathPlayer, const FVector DeathLocation, const AAccelByteWarsPlayerState* Killer);

	void UpdateSpectatingTextEffect(float DeltaTime);
	void CheckSpectatingText();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Timer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Spectating;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_LeftPanel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UHorizontalBox* Hb_RightPanel;

	/* Used to tell the game what size the visible HUD is */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_VisibleBorder;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* W_PreGameCountdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* W_NotEnoughPlayerCountdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCountdownWidget* W_SimulateServerCrashCountdown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPushNotificationWidget* W_KillFeed;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UHUDWidgetEntry> HUDWidgetEntryClass;

	UPROPERTY()
	AAccelByteWarsInGameGameState* GameState;
	
	TMap<int32, TWeakObjectPtr<UHUDWidgetEntry>> HUDWidgetEntries;

	FDelegateHandle OnPreGameCountdownFinishedDelegateHandle;
	FDelegateHandle OnNotEnoughPlayerCountdownFinishedDelegateHandle;
	FDelegateHandle OnSimulateServerCrashCountdownFinishedDelegateHandle;

	float SpectatingTextVisibleRunningTime = 0.0f;
};
