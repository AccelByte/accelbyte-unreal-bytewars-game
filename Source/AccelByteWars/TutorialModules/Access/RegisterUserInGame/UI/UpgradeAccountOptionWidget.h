// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameSubsystem.h"
#include "UpgradeAccountOptionWidget.generated.h"

class UCommonButtonBase;
class UAccelByteWarsGameInstance;

UCLASS()
class ACCELBYTEWARS_API UUpgradeAccountOptionWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

// @@@SNIPSTART UpgradeAccountOptionWidget.h-protected
// @@@MULTISNIP UpgradeAccountOptionUI {"selectedLines": ["1", "14-18"]}
// @@@MULTISNIP UpgradeAccountOptionDeclaration {"selectedLines": ["1", "7-8"]}
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const;

	void ProceedToUpgradeAccount();
	void SkipUpgradeAccount();

	UAccelByteWarsGameInstance* GameInstance;

	URegisterUserInGameSubsystem* RegisterUserInGameSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Upgrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Skip;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> UpgradeAccountWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> MainMenuWidgetClass;
// @@@SNIPEND
};
