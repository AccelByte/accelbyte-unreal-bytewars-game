// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-1/TutorialModuleOnlineUtility.h"
#include "Core/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"
#include "OnlineSubsystemUtils.h"
#include "Core/AccelByteRegistry.h"
#include "AccelByteUe4SdkModule.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsTutorialModuleOnlineUtility);

UTutorialModuleOnlineUtility::UTutorialModuleOnlineUtility()
{
    CheckForEnvironmentConfigOverride();
}

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

void UTutorialModuleOnlineUtility::CheckForEnvironmentConfigOverride()
{
    FString CmdArgs = FCommandLine::Get();
    if (!CmdArgs.Contains(TEXT("-TARGET_ENV="), ESearchCase::IgnoreCase))
    {
        return;
    }

    FString EnvironmentStr;
    FParse::Value(FCommandLine::Get(), TEXT("-TARGET_ENV="), EnvironmentStr);
    if (!EnvironmentStr.IsEmpty())
    {
        ESettingsEnvironment ABEnvironment = ConvertStringEnvToAccelByteEnv(EnvironmentStr);
        IAccelByteUe4SdkModuleInterface::Get().SetEnvironment(ABEnvironment);
    }
    else
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Cannot change target environment. Desired target environment cannot be found from command line."));
    }
}

ESettingsEnvironment UTutorialModuleOnlineUtility::ConvertStringEnvToAccelByteEnv(const FString& EnvironmentStr)
{
    if (EnvironmentStr.Equals(FString("Development"), ESearchCase::IgnoreCase))
    {
        return ESettingsEnvironment::Development;
    }
    else if (EnvironmentStr.Equals(FString("Certification"), ESearchCase::IgnoreCase))
    {
        return ESettingsEnvironment::Certification;
    }
    else if (EnvironmentStr.Equals(FString("Production"), ESearchCase::IgnoreCase))
    {
        return ESettingsEnvironment::Production;
    }
    else
    {
        return ESettingsEnvironment::Default;
    }
}

ESettingsEnvironment UTutorialModuleOnlineUtility::ConvertOSSEnvToAccelByteEnv(const EOnlineEnvironment::Type& Environment)
{
    switch (Environment)
    {
        case EOnlineEnvironment::Type::Development:
            return ESettingsEnvironment::Development;
        case EOnlineEnvironment::Type::Certification:
            return ESettingsEnvironment::Certification;
        case EOnlineEnvironment::Type::Production:
            return ESettingsEnvironment::Production;
        case EOnlineEnvironment::Type::Unknown:
        default:
            return ESettingsEnvironment::Default;
    }
}