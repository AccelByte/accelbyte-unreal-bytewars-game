// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OnlineSettings/RegionPreferencesEssentials/RegionPreferencesSubsystem_Starter.h"
#include "RegionPreferencesWidget_Starter.generated.h"


class UListView;
class UTextBlock;
class UAccelByteWarsButtonBase;
class UAccelByteWarsWidgetSwitcher;
UCLASS()

class ACCELBYTEWARS_API URegionPreferencesWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
#pragma region "Tutorial"
	// put your code here
#pragma endregion 
	URegionPreferencesSubsystem_Starter* RegionPreferencesSubsystem;

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
};
