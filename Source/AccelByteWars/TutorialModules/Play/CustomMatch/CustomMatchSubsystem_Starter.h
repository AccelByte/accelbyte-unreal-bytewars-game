// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionModels.h"
#include "CustomMatchSubsystem_Starter.generated.h"

class UAccelByteWarsOnlineSessionBase;

UCLASS()
class ACCELBYTEWARS_API UCustomMatchSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
#pragma region "Tutorial"
	// Insert your codes here
#pragma endregion

private:
#pragma region "Tutorial"
	// Insert your codes here
#pragma endregion
};
