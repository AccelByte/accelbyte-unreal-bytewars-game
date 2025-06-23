// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChallengeEssentialsSubsystem_Starter.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"
#include "OnlineSubsystemUtils.h"

AccelByte::Api::ChallengePtr UChallengeEssentialsSubsystem_Starter::GetChallengeApi() const
{
    AccelByte::FApiClientPtr ApiClient = UTutorialModuleOnlineUtility::GetApiClient(this);
    if (!ApiClient)
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("AccelByte API Client is invalid."));
        return nullptr;
    }

    return ApiClient->GetChallengeApi().Pin();
}

FOnlineStoreV2AccelBytePtr UChallengeEssentialsSubsystem_Starter::GetStoreInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_CHALLENGE_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineStoreV2AccelByte>(Subsystem->GetStoreV2Interface());
}

#pragma region Module Challenge Essentials Function Definitions
// TODO: Add your function definitions here.
#pragma endregion
