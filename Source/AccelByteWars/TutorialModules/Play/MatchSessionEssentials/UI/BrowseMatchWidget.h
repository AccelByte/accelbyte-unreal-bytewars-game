// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "BrowseMatchWidget.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;
class UAccelByteWarsWidgetList;
class UAccelByteWarsWidgetSwitcher;
class UMatchSessionEssentialsSubsystem;
class UListView;

UCLASS(Abstract)
class ACCELBYTEWARS_API UBrowseMatchWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnActivated() override;

	enum class EContentType : uint8
	{
		BROWSE_LOADING = 0,
		BROWSE_EMPTY,
		BROWSE_NOT_EMPTY,
		BROWSE_ERROR,
		JOIN_LOADING,
		JOIN_ERROR
	};

	UListView* GetListViewWidgetComponent() const { return Lv_Sessions; }
	UAccelByteWarsWidgetSwitcher* GetJoiningWidgetComponent() const { return Ws_Joining; }

	void SetLoadingMessage(const FText& Text, const bool bBrowse, const bool bEnableCancelButton = false) const;
	void SetErrorMessage(const FText& Text, const bool bBrowse) const;
	void SwitchContent(const EContentType Type);

protected:
	virtual void NativeOnDeactivated() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	float CameraTargetY = 600.f;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_Root;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Browse_Outer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Joining_Outer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Browse_Content;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Joining;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UListView* Lv_Sessions;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Joining_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_ActionButtonsOuter;
};
