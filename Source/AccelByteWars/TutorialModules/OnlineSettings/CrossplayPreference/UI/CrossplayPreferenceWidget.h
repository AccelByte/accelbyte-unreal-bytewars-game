// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "CrossplayPreferenceWidget.generated.h"

class UCrossplayPreferenceSubsystem;
class UAccelByteWarsWidgetSwitcher;
class UCommonButtonBase;
class UOptionListEntry_Toggler;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCrossplayPreferenceWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY()
	UCrossplayPreferenceSubsystem* Subsystem;

	UFUNCTION()
	void UpdateCrossplayPreference();

	void ResetUI() const;

#pragma region "UI"
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Outer;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Toggler* W_OptionEnabled;
#pragma endregion 
};
