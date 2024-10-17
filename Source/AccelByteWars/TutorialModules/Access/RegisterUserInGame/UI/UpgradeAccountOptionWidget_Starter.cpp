// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "UpgradeAccountOptionWidget_Starter.h"
#include "TutorialModules/Access/RegisterUserInGame/RegisterUserInGameLog.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "CommonButtonBase.h"

void UUpgradeAccountOptionWidget_Starter::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	RegisterUserInGameSubsystem = GameInstance->GetSubsystem<URegisterUserInGameSubsystem_Starter>();
	ensure(RegisterUserInGameSubsystem);

	// TODO: Add your function call here.
}

void UUpgradeAccountOptionWidget_Starter::NativeOnActivated()
{
	Super::NativeOnActivated();

	// TODO: Add your function call here.
}

void UUpgradeAccountOptionWidget_Starter::NativeOnDeactivated()
{
	Btn_Upgrade->OnClicked().Clear();
	Btn_Skip->OnClicked().Clear();

	Super::NativeOnDeactivated();
}

UWidget* UUpgradeAccountOptionWidget_Starter::NativeGetDesiredFocusTarget() const
{
	return Btn_Upgrade;
}

#pragma region Module Register User In-Game Function Definitions
// TODO: Add your function definitions here.
#pragma endregion
