// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameSubsystem_Starter.h"
#include "UpgradeAccountOptionWidget_Starter.generated.h"

class UCommonButtonBase;
class UAccelByteWarsGameInstance;

UCLASS()
class ACCELBYTEWARS_API UUpgradeAccountOptionWidget_Starter : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const;

#pragma region Module Register User In-Game Function Declarations
	// TODO: Add your protected function declarations here.
#pragma endregion

	UAccelByteWarsGameInstance* GameInstance;

	URegisterUserInGameSubsystem_Starter* RegisterUserInGameSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Upgrade;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UCommonButtonBase* Btn_Skip;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> UpgradeAccountWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAccelByteWarsActivatableWidget> MainMenuWidgetClass;
};
