// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-1/TutorialModuleOnlineUtility.h"
#include "Core/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteRegistry.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsTutorialModuleOnlineUtility);

bool UTutorialModuleOnlineUtility::IsAccelByteSDKInitialized(const UObject* Target)
{
    bool IsOSSEnabled = true;
    bool IsSDKCredsEmpty = false;

    // Check AccelByte Subsystem.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(Target->GetWorld());
    if (!ensure(Subsystem) || !Subsystem->IsEnabled())
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("AccelByte SDK and OSS is not valid."));
        IsOSSEnabled = false;
    }

    if (IsRunningDedicatedServer()) 
    {
        // Check server credentials.
        AccelByte::ServerSettings ServerCreds = AccelByte::FRegistry::ServerSettings;
        if (ServerCreds.ClientId.IsEmpty() || ServerCreds.ClientSecret.IsEmpty() ||
            ServerCreds.Namespace.IsEmpty() || ServerCreds.BaseUrl.IsEmpty())
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Server creds are empty or not filled properly. Please check your AccelByte SDK settings configuration."));
            IsSDKCredsEmpty = true;
        }
    }
    else 
    {
        // Check client credentials.
        AccelByte::Settings ClientCreds = AccelByte::FRegistry::Settings;
        if (ClientCreds.ClientId.IsEmpty() || ClientCreds.Namespace.IsEmpty() ||
            ClientCreds.BaseUrl.IsEmpty())
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Client creds are empty or not filled properly. Please check your AccelByte SDK settings configuration."));
            IsSDKCredsEmpty = true;
        }
    }

    return IsOSSEnabled && !IsSDKCredsEmpty;
}