// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "Core/UI/Components/Countdown/CountdownWidget.h"
#include "HUDWidget.generated.h"

class UHUDWidgetEntry;
/**
 * 
 */
UCLASS()
class ACCELBYTEWARS_API UHUDWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/**
	 * @brief Set HUD entries value
	 * @param Value Value to be displayed
	 * @param Index Team's index. Currently, this HUD only supports 4 team (0 - 3)
	 * @param BoxIndex Box index. 0 = left; 1 = middle; 2 = right
	 */
	UFUNCTION(BlueprintCallable)
	void SetValue(const FString Value, const int32 Index, const int32 BoxIndex);

	/**
	 * @brief Change the color of HUD entry. Will only be executed when Color is not the same with current HUD entry's color
	 * @param Index Team's index. Currently, this HUD only supports 4 team (0 - 3)
	 * @param Color Team's color
	 */
	UFUNCTION(BlueprintCallable)
	void SetColorChecked(const int32 Index, const FLinearColor Color);

	/**
	 * @brief Set Time Left value
	 * @param TimeLeft Time Left
	 */
	UFUNCTION(BlueprintCallable)
	void SetTimerValue(const float TimeLeft);

private:
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
	UCountdownWidget* Widget_Countdown;

	UPROPERTY()
	AAccelByteWarsGameStateBase* ByteWarsGameState;

	UFUNCTION()
	ECountdownState SetCountdownState();

	UFUNCTION()
	int UpdateCountdownValue();

	UFUNCTION()
	void OnCountdownFinished();
};
