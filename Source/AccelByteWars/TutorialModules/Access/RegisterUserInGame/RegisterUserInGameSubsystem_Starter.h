// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "RegisterUserInGameModels.h"
#include "RegisterUserInGameLog.h"
#include "RegisterUserInGameSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API URegisterUserInGameSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool IsAllowUpgradeAccount();

#pragma region Module Register User In-Game Function Declarations
	// TODO: Add your public function declarations here.
#pragma endregion

protected:
#pragma region Module Register User In-Game Function Declarations
	// TODO: Add your protected function declarations here.
#pragma endregion

private:
	void GetUpgradeAccountConfig();

	bool bAllowUpgradeAccount = false;
};
