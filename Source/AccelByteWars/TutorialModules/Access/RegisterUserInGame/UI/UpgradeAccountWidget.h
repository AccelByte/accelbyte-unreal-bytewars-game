// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameSubsystem.h"
#include "UpgradeAccountWidget.generated.h"

class UTextBlock;
class UEditableText;
class UCommonButtonBase;
class UAccelByteWarsGameInstance;

UCLASS()
class ACCELBYTEWARS_API UUpgradeAccountWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

// @@@SNIPSTART UpgradeAccountWidget.h-protected
// @@@MULTISNIP UpgradeAccountUI {"selectedLines": ["1", "14-33"]}
// @@@MULTISNIP UpgradeAccount {"selectedLines": ["1", "6"]}
protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const;

	void UpgradeAccount();

	void ToggleWarningText(bool bShow, const FText& Message = FText::GetEmpty());
	void ProceedToVerifyAccount();

	UAccelByteWarsGameInstance* GameInstance;
	URegisterUserInGameSubsystem* RegisterUserInGameSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Warning;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_Username;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_DisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_Email;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_Password;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_RetypePassword;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Upgrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> VerifyAccountWidgetClass;
// @@@SNIPEND

private:
	bool bResetInputOnDeactivated = true;
};
