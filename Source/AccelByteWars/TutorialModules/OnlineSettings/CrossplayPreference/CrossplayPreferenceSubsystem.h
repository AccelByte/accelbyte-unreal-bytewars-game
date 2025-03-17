// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "CrossplayerPreferenceModels.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "Core/AccelByteApiClient.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CrossplayPreferenceSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UCrossplayPreferenceSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	void RetrieveCrossplayPreference(const FUniqueNetIdPtr PlayerNetId, FOnRetrieveCrossplayPreferenceCompleted OnComplete);
	void UpdateCrossplayPreference(const FUniqueNetIdPtr PlayerNetId, bool bEnabled, FOnUpdateCrossplayPreferenceCompleted OnComplete);

private:
	FOnlineSessionV2AccelBytePtr SessionInterface;
	IOnlineIdentityPtr IdentityInterface;
	FOnlineSessionV2AccelBytePlayerAttributes PlayerAttributes;
};
