// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameSubsystem_Starter.h"
#include "VerifyAccountWidget_Starter.generated.h"

class UTextBlock;
class UEditableText;
class UCommonButtonBase;
class UAccelByteWarsButtonBase;
class UAccelByteWarsWidgetSwitcher;
class UAccelByteWarsGameInstance;

UCLASS()
class ACCELBYTEWARS_API UVerifyAccountWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

public:
	void SetAccountToVerify(const FUpgradeAccountData& InUpgradeAccountData);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime);
	virtual UWidget* NativeGetDesiredFocusTarget() const;

#pragma region Module Register User In-Game Function Declarations
	// TODO: Add your protected function declarations here.
#pragma endregion

	void ToggleWarningText(bool bShow, const FText& Message = FText::GetEmpty());
	void OpenToMainMenu();

	UAccelByteWarsGameInstance* GameInstance;

	FUpgradeAccountData UpgradeAccountData;
	URegisterUserInGameSubsystem_Starter* RegisterUserInGameSubsystem;

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

private:
	UPROPERTY(EditDefaultsOnly)
	uint32 RequestVerificationCodeCooldown = 60;

	float RequestVerificationCodeTimer = 0.0f;
};
