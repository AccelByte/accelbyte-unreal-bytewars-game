// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "TutorialModules/PlayingWithParty/PlayWithPartyLog.h"
#include "TutorialModules/PlayingWithParty/PlayWithPartyModels.h"
#include "PlayWithPartySubsystem_Starter.generated.h"

class UAccelByteWarsOnlineSessionBase;
class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPlayWithPartySubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

protected:
#pragma region "Playing With Party Module Function Declarations"
	// TODO: Add your playing with party module protected function declarations here.
#pragma endregion

	UAccelByteWarsOnlineSessionBase* GetOnlineSession() const;
	FOnlineSessionV2AccelBytePtr GetSessionInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubystem();
};
