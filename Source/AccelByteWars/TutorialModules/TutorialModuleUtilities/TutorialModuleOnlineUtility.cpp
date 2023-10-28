// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModuleOnlineUtility.h"
#include "Core/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

#include "Core/AccelByteRegistry.h"
#include "AccelByteUe4SdkModule.h"

#include "Access/AuthEssentials/AuthEssentialsModels.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsTutorialModuleOnlineUtility);

UTutorialModuleOnlineUtility::UTutorialModuleOnlineUtility()
{
    CheckForEnvironmentConfigOverride();

    CheckForSDKConfigOverride();

    CheckForDedicatedServerVersionOverride();

    // Trigger to get general predefined argument.
    FTUEArgumentModel::OnGetPredefinedArgument.BindUObject(this, &ThisClass::GetFTUEPredefinedArgument);

    // Save general logged-in player information.
    UAuthEssentialsModels::OnLoginSuccessDelegate.AddWeakLambda(this, [this](const APlayerController* PC)
    {
        const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
        if (!Subsystem)
        {
            return;
        }

        const FOnlineIdentityAccelBytePtr IdentityInterface = 
            StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
        if (!IdentityInterface)
        {
            return;
        }

        const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
        if (!LocalPlayer)
        {
            return;
        }

        const FUniqueNetIdAccelByteUserPtr UserABId = 
            StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId());
        if (!UserABId) 
        {
            return;
        }

        // Save player's AccelByte user id.
        CurrentPlayerUserIdStr = UserABId->GetAccelByteId();

        // Save player's display name.
        TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(UserABId.ToSharedRef().Get());
        if (UserAccount) 
        {
            CurrentPlayerDisplayName = UserAccount->GetDisplayName();
        }
        if (CurrentPlayerDisplayName.IsEmpty())
        {
            CurrentPlayerDisplayName = GetUserDefaultDisplayName(CurrentPlayerUserIdStr);
        }
    });
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

bool UTutorialModuleOnlineUtility::GetIsServerUseAMS()
{
    // Check launch param
    bool bUseAMS = FParse::Param(FCommandLine::Get(), TEXT("-ServerUseAMS"));
	
    // check DefaultEngine.ini next
    if (!bUseAMS)
    {
        FString Config;
        GConfig->GetBool(TEXT("/ByteWars/TutorialModule.DSEssentials"), TEXT("bServerUseAMS"), bUseAMS, GEngineIni);
    }

    return bUseAMS;
}

void UTutorialModuleOnlineUtility::CheckForDedicatedServerVersionOverride()
{
    DedicatedServerVersionOverride = FString("");

    bool bIsOverridden = false;

    // Check dedicated server (DS) version override from launch parameter.
    const FString CmdArgs = FCommandLine::Get();
    const FString CmdStr = FString("-OverrideDSVersion=");
    bool bValidCmdValue = false;
    if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
    {
        FString CmdValue;
        FParse::Value(*CmdArgs, *CmdStr, CmdValue);

        if (!CmdValue.IsEmpty())
        {
            if (CmdValue.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase))
            {
                bValidCmdValue = true;
                bIsOverridden = true;
            }
            else if (CmdValue.Equals(TEXT("FALSE"), ESearchCase::IgnoreCase))
            {
                bValidCmdValue = true;
                bIsOverridden = false;
            }
        }

        if (bValidCmdValue)
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log,
                TEXT("Launch param set the override DS version config to %s."),
                bIsOverridden ? TEXT("TRUE") : TEXT("FALSE"));
        }
        else
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to set enable/disable the override DS version config using launch param. Empty or invalid value."));
        }
    }

    // Check dedicated server (DS) version override from DefaultEngine.ini
    const FString IniSectionPath = FString("/ByteWars/TutorialModule.DSEssentials");
    const FString IniConfig = FString("bOverrideDSVersion");
    if (!bValidCmdValue)
    {
        GConfig->GetBool(*IniSectionPath, *IniConfig, bIsOverridden, GEngineIni);

        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log,
            TEXT("DefaultEngine.ini set the override DS version config to %s."),
            bIsOverridden ? TEXT("TRUE") : TEXT("FALSE"));
    }

    // Abort if not overridden.
    if (!bIsOverridden)
    {
        return;
    }

    // Set dedicated server version based on project version (could be empty if not yet set in DefaultGame.ini).
    FString ProjectVersionStr;
    const FString ProjectVerSectionPath = FString("/Script/EngineSettings.GeneralProjectSettings");
    const FString ProjectVerConfig = FString("ProjectVersion");
    GConfig->GetString(*ProjectVerSectionPath, *ProjectVerConfig, ProjectVersionStr, GGameIni);

    UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("DS version is overridden to: %s"), *ProjectVersionStr);

    DedicatedServerVersionOverride = ProjectVersionStr;
}

FString UTutorialModuleOnlineUtility::GetDedicatedServerVersionOverride()
{
    return DedicatedServerVersionOverride;
}

void UTutorialModuleOnlineUtility::CheckForSDKConfigOverride()
{
    bool bIsOverridden = false;

    // Try to override client SDK config from launch param.
    bIsOverridden = OverrideSDKConfigFromLaunchParam() || bIsOverridden;
    
    // Try to override server SDK config from launch param.
    bIsOverridden = OverrideSDKConfigFromLaunchParam(true) || bIsOverridden;

    /* Overridden SDK config is placed at the default environment.
     * Thus, update the environment to use the default environment.*/
    if (bIsOverridden) 
    {
        IAccelByteUe4SdkModuleInterface::Get().SetEnvironment(ESettingsEnvironment::Default);
    }
}

bool UTutorialModuleOnlineUtility::OverrideSDKConfigFromLaunchParam(const bool bIsServer)
{
    const FString ClientSectionPath = FString("/Script/AccelByteUe4Sdk.AccelByteSettings");
    const FString ServerSectionPath = FString("/Script/AccelByteUe4Sdk.AccelByteServerSettings");

    const FString ClientPrefix = FString("-Client_");
    const FString ServerPrefix = FString("-Server_");

    TArray<FString> ConfigKeys =
    {
        FString("ClientId"),
        FString("ClientSecret"),
        FString("Namespace"),
        FString("PublisherNamespace"),
        FString("BaseUrl"),
        FString("RedirectURI")
    };

    // Check launch param and override the SDK config.
    bool bIsOverridden = false;
    const FString CmdArgs = FCommandLine::Get();
    const FString ConfigPrefix = bIsServer ? ServerPrefix : ClientPrefix;
    const FString SectionPath = bIsServer ? ServerSectionPath : ClientSectionPath;
    for (const FString& Config : ConfigKeys)
    {
        const FString CmdStr = FString::Printf(TEXT("%s%s="), *ConfigPrefix, *Config, false);
        if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
        {
            FString CmdValue;
            FParse::Value(*CmdArgs, *CmdStr, CmdValue);
            if (CmdValue.IsEmpty())
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(
                    Warning, 
                    TEXT("Unable to override %s SDK config %s using launch param. Empty or invalid value."), 
                    bIsServer ? TEXT("Server") : TEXT("Client"),
                    *Config);
                continue;
            }

            GConfig->SetString(*SectionPath, *Config, *CmdValue, GEngineIni);
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(
                Log, 
                TEXT("%s SDK config %s is overridden by launch param to %s"), 
                bIsServer ? TEXT("Server") : TEXT("Client"), 
                *Config, 
                *CmdValue);

            bIsOverridden = true;
        }
    }
    
    return bIsOverridden;
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
        // Set AccelByte target environment.
        const ESettingsEnvironment ABEnvironment = ConvertStringEnvToAccelByteEnv(EnvironmentStr);
        IAccelByteUe4SdkModuleInterface::Get().SetEnvironment(ABEnvironment);
        
        // Check current AccelByte target environment.
        const ESettingsEnvironment CurrentABEnvironment = IAccelByteUe4SdkModuleInterface::Get().GetSettingsEnvironment();
        if (CurrentABEnvironment != ESettingsEnvironment::Default)
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Target environment is set by launch param to %s"), *ConvertAccelByteEnvToStringEnv(CurrentABEnvironment));
        }
        else
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Cannot set target environment by launch param. Target environment %s is not valid. Fallback to the default environment."), *EnvironmentStr);
        }
    }
    else
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Cannot set target environment by launch param. Desired target environment cannot be found. Fallback to the default environment."));
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

FString UTutorialModuleOnlineUtility::ConvertAccelByteEnvToStringEnv(const ESettingsEnvironment& Environment)
{
    switch(Environment) 
    {
        case ESettingsEnvironment::Development:
            return FString("Development");
        case ESettingsEnvironment::Certification:
            return FString("Certification");
        case ESettingsEnvironment::Production:
            return FString("Production");
        case ESettingsEnvironment::Default:
        default:
            return FString("Default");
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

FString UTutorialModuleOnlineUtility::GetFTUEPredefinedArgument(const FTUEPredifinedArgument Keyword)
{
    FString Result = FString("");

    if (Keyword == FTUEPredifinedArgument::PLAYER_ID) 
    {
        Result = CurrentPlayerUserIdStr;
    }
    else if (Keyword == FTUEPredifinedArgument::PLAYER_DISPLAY_NAME)
    {
        Result = CurrentPlayerDisplayName;
    }
    else if (Keyword == FTUEPredifinedArgument::GAME_SESSION_ID) 
    {
        if (FNamedOnlineSession* Session = GetOnlineSession(NAME_GameSession, this))
        {
            Result = Session->GetSessionIdStr();
        }
    }
    else if (Keyword == FTUEPredifinedArgument::PARTY_SESSION_ID)
    {
        if (FNamedOnlineSession* Session = GetOnlineSession(NAME_PartySession, this))
        {
            Result = Session->GetSessionIdStr();
        }
    }
    else if (Keyword == FTUEPredifinedArgument::DEDICATED_SERVER_ID) 
    {
        Result = GetDedicatedServer(this).Server.Pod_name;
    }

    return Result;
}

FNamedOnlineSession* UTutorialModuleOnlineUtility::GetOnlineSession(const FName SessionName, const UObject* Context)
{
    if (!Context) 
    {
        return nullptr;
    }

    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(Context->GetWorld());
    if (!Subsystem)
    {
        return nullptr;
    }

    const FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
    if (!SessionInterface)
    {
        return nullptr;
    }

    return SessionInterface->GetNamedSession(SessionName);
}

FAccelByteModelsV2GameSessionDSInformation UTutorialModuleOnlineUtility::GetDedicatedServer(const UObject* Context)
{
    FAccelByteModelsV2GameSessionDSInformation DSInformation{};

    if (!Context)
    {
        return DSInformation;
    }

    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(Context->GetWorld());
    if (!Subsystem)
    {
        return DSInformation;
    }

    FOnlineSessionV2AccelBytePtr SessionInterface = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
    if (!SessionInterface)
    {
        return DSInformation;
    }

    FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
    if (!Session) 
    {
        return DSInformation;
    }

    TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
    if (!SessionInfo) 
    {
        return DSInformation;
    }

    TSharedPtr<FAccelByteModelsV2GameSession> SessionData = SessionInfo->GetBackendSessionDataAsGameSession();
    if (!SessionData)
    {
        return DSInformation;
    }

    DSInformation = SessionData->DSInformation;

    return DSInformation;
}
