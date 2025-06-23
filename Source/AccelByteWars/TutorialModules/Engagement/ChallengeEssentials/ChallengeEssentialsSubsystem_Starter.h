// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"

#include "Api/AccelByteChallengeApi.h"
#include "OnlineStoreInterfaceV2AccelByte.h"
#include "OnlineSubsystemAccelByte.h"

#include "ChallengeEssentialsModels.h"
#include "ChallengeEssentialsLog.h"
#include "ChallengeEssentialsSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UChallengeEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

#pragma region Module Challenge Essentials Function Declarations
public:
	// TODO: Add your public function declarations here.
#pragma endregion

protected:
	AccelByte::Api::ChallengePtr GetChallengeApi() const;
	FOnlineStoreV2AccelBytePtr GetStoreInterface() const;

#pragma region Module Challenge Essentials Function Declarations
private:
	// TODO: Add your private function declarations here.
#pragma endregion
};
