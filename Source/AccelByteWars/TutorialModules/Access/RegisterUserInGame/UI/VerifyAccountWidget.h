// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameSubsystem.h"
#include "VerifyAccountWidget.generated.h"

class UTextBlock;
class UEditableText;
class UCommonButtonBase;
class UAccelByteWarsButtonBase;
class UAccelByteWarsWidgetSwitcher;
class UAccelByteWarsGameInstance;

UCLASS()
class ACCELBYTEWARS_API UVerifyAccountWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void SetAccountToVerify(const FUpgradeAccountData& InUpgradeAccountData);

// @@@SNIPSTART VerifyAccountWidget.h-protected
// @@@MULTISNIP VerifyAccountUI {"selectedLines": ["1", "23-36"]}
// @@@MULTISNIP VerifyAccountDeclaration {"selectedLines": ["1", "8-13"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	virtual UWidget* NativeGetDesiredFocusTarget() const;

	void SendVerificationCode(const bool bForceResend = false);
	void UpgradeAndVerifyAccount();
	
	void StartRequestVerificationCodeCooldown();
	void UpdateRequestVerificationCodeCooldown(float DeltaTime);
	void ResetRequestVerificationCodeCooldown();

	void ToggleWarningText(bool bShow, const FText& Message = FText::GetEmpty());
	void OpenToMainMenu();

	UAccelByteWarsGameInstance* GameInstance;

	FUpgradeAccountData UpgradeAccountData;
	URegisterUserInGameSubsystem* RegisterUserInGameSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Warning;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UEditableText* Edt_VerificationCode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsWidgetSwitcher* Ws_VerifyAccount;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UAccelByteWarsButtonBase* Btn_Resend;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Verify;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Back;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> MainMenuWidgetClass;
// @@@SNIPEND

private:
	UPROPERTY(EditDefaultsOnly)
	uint32 RequestVerificationCodeCooldown = 60;

	float RequestVerificationCodeTimer = 0.0f;
};
