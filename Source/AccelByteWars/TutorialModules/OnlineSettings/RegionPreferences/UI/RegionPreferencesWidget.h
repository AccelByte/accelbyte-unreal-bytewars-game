// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem.h"
#include "RegionPreferencesWidget.generated.h"

class UListView;
class UTextBlock;
class UAccelByteWarsButtonBase;
class UAccelByteWarsWidgetSwitcher;
UCLASS(Abstract)

class ACCELBYTEWARS_API URegionPreferencesWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
private:
	void ShowWarningText();
	void UpdateWarningText(float DeltaTime);
	void SetupUI();
	void OnRefreshButtonClicked();
	void OnRefreshRegionComplete(bool bWasSucceed);
	
	URegionPreferencesSubsystem* RegionPreferencesSubsystem;

#pragma region "UI"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Regions;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Regions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Text_Warning;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Refresh;
#pragma endregion

	float WarningTextRemainingTime;
};
