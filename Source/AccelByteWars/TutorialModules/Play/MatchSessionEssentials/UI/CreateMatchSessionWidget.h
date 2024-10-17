// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "CreateMatchSessionWidget.generated.h"

class UCommonButtonBase;
class UWidgetSwitcher;
class UTextBlock;
class UAccelByteWarsWidgetSwitcher;

UCLASS(Abstract)
class ACCELBYTEWARS_API UCreateMatchSessionWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

// @@@SNIPSTART CreateMatchSessionWidget.h-public
// @@@MULTISNIP GetSelectedGameModeType {"selectedLines": ["1", "5"]}
public:
	virtual void NativeOnActivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	EGameModeType GetSelectedGameModeType() const { return SelectedGameModeType; }
	UAccelByteWarsWidgetSwitcher* GetProcessingWidgetComponent() const { return Ws_Processing; }

	enum class EContentType : uint8
	{
		SELECT_GAMEMODE = 0,
		SELECT_NETWORKTYPE = 1,
		LOADING,
		ERROR
	};

	void SetLoadingMessage(const FText& Message, const bool bEnableCancelButton = true) const;
	void SetErrorMessage(const FText& Message, const bool bShowRetryButton = true) const;
	void SwitchContent(const EContentType ContentType);
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionWidget.h-protected
protected:
	virtual void NativeOnDeactivated() override;

	virtual UWidget* NativeGetDesiredFocusTarget() const override;

	UFUNCTION()
	void SetSelectedGameMode(EGameModeType GameModeType);

	float CameraTargetY = 600.f;
// @@@SNIPEND

// @@@SNIPSTART CreateMatchSessionWidget.h-private
// @@@MULTISNIP CreateMatchSessionUISwitcher {"selectedLines": ["1", "6-7", "18-19"]}
// @@@MULTISNIP SelectGameModeUI {"selectedLines": ["1", "33-37"]}
private:
	EGameModeType SelectedGameModeType = EGameModeType::FFA;

	UWidget* DesiredFocusTargetButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ContentOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_SelectGameModeType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_SelectGameModeNetworkType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_Processing;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_Processing;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UPanelWidget* W_SelectGameModeNetworkTypeButtonOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_GameModeType_BackToCreateSession;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_ServerType_BackToCreateSession;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Error_BackToCreateSession;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Elimination;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_TeamDeathMatch;
// @@@SNIPEND
};
