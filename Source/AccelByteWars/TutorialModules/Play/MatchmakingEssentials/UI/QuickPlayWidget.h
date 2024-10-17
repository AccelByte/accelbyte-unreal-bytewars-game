// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "QuickPlayWidget.generated.h"

class UWidgetSwitcher;
class UCommonButtonBase;

UCLASS(Abstract)
class ACCELBYTEWARS_API UQuickPlayWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	enum class EContentType : uint8
	{
		SELECTGAMEMODE = 0,
		SELECTSERVERTYPE
	};

	EGameModeType GetSelectedGameModeType() const;

	void SwitchContent(EContentType State);

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;

private:
	void SelectGameMode(EGameModeType GameModeType);

	EGameModeType SelectedGameModeType = EGameModeType::TDM;
	float CameraTargetY = 600.f;

	UPROPERTY()
	UWidget* DesiredFocusTargetButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidgetSwitcher* Ws_ContentOuter;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_SelectGameMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UWidget* W_SelectServerType;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Elimination;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_TeamDeathMatch;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_SelectGameMode_Back;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_SelectServerType_Back;
};