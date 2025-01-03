// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlinePresenceInterfaceAccelByte.h"
#include "PresenceEssentialsLog.h"
#include "PresenceEssentialsModels.h"
#include "PresenceEssentialsSubsystem_Starter.generated.h"

class FOnlineFriendsAccelByte;
class FOnlineIdentityAccelByte;
class UAccelByteWarsOnlineSessionBase;

UCLASS()
class ACCELBYTEWARS_API UPresenceEssentialsSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
#pragma region Module Presence Essentials Function Declarations
	// TODO: Add your public function declarations here.
#pragma endregion

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

protected:
#pragma region Module Presence Essentials Function Declarations
	// TODO: Add your protected function declarations here.
#pragma endregion

	FOnlinePresenceAccelBytePtr GetPresenceInterface() const;
	TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> GetFriendsInterface() const;
	TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> GetIdentityInterface() const;
	TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> GetSessionInterface() const;
	UAccelByteWarsOnlineSessionBase* GetOnlineSession() const;

private:
#pragma region Module Presence Essentials Function Declarations
	// TODO: Add your private function declarations here.
#pragma endregion

	FUniqueNetIdPtr GetPrimaryPlayerUserId();
};
