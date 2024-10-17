// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModuleOnlineUtility.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

#include "Core/AccelByteRegistry.h"
#include "AccelByteUe4SdkModule.h"

#include "Access/AuthEssentials/AuthEssentialsModels.h"

#include "Internationalization/Internationalization.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsTutorialModuleOnlineUtility);

UTutorialModuleOnlineUtility::UTutorialModuleOnlineUtility()
{
    // Try to check if use version checker.
    CheckUseVersionChecker();

    // Try to override environment config.
    CheckForEnvironmentConfigOverride();

    // Try to override SDK config.
    OverrideSDKConfig(CheckForSDKConfigOverride(false), CheckForSDKConfigOverride(true));

    // Try to override game version.
    CheckForGameVersionOverride();

    // Try to override client version attribute.
    CheckForClientVersionOverride();

    // Try to override server version.
    CheckForDedicatedServerVersionOverride();

    // Try to override Session Template and Match Pool.
    CheckSessionTemplateAndMatchPoolOverride();

    // Initialize AGS Starter if valid.
    IntializeAGSStaterIfValid();

    // Trigger to get general predefined argument.
    FServiceArgumentModel::OnGetPredefinedArgument.BindUObject(this, &ThisClass::GetServicePredefinedArgument);

    // Trigger to execute predefined service validator for FTUE dialogues.
    FFTUEDialogueModel::OnPredefinedValidationDelegate.BindUObject(this, &ThisClass::ExecutePredefinedServiceForFTUE);

    // Trigger to execute predefined service validator for Widget Validators.
    FWidgetValidator::OnPredefinedServiceValidatorExecutedDelegate.BindUObject(this, &ThisClass::ExecutePredefinedServiceForWidgetValidator);

    // Cache general information after login.
    UAuthEssentialsModels::OnLoginSuccessDelegate.AddUObject(this, &ThisClass::CacheGeneralInformation);

    // On player added to team but the display name is empty, set user default display name.
    AAccelByteWarsGameState::OnSetDefaultDisplayName.BindWeakLambda(this, [](const FUniqueNetId& UserId)
    {
        return GetUserDefaultDisplayName(UserId);
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
    bool bUseAMS = true; // default is true

    // Check launch param. Prioritize launch param.
    FString UseAMSString;
    if (FParse::Value(FCommandLine::Get(), TEXT("-bServerUseAMS="), UseAMSString))
    {
        bUseAMS = !UseAMSString.Equals("false", ESearchCase::Type::IgnoreCase);
    }
    // check DefaultEngine.ini next
    else
    {
        GConfig->GetBool(TEXT("/ByteWars/TutorialModule.DSEssentials"), TEXT("bServerUseAMS"), bUseAMS, GEngineIni);
    }

    return bUseAMS;
}

void UTutorialModuleOnlineUtility::OverrideSDKConfig(const TMap<FString, FString>& ClientConfigs, const TMap<FString, FString>& ServerConfigs)
{
    // Abort if no configs to be overridden.
    if (ClientConfigs.IsEmpty() && ServerConfigs.IsEmpty()) 
    {
        return;
    }

    TMap<FString, FString> OriginalClientConfigs, OriginalServerConfigs;

    const FString ClientSectionPath = FString("/Script/AccelByteUe4Sdk.AccelByteSettings");
    const FString ServerSectionPath = FString("/Script/AccelByteUe4Sdk.AccelByteServerSettings");

    // Try to override sdk config (client).
    for (auto& ConfigPair : ClientConfigs)
    {
        // Skip if the current value is the same.
        FString LastValue;
        GConfig->GetString(*ClientSectionPath, *ConfigPair.Key, LastValue, GEngineIni);
        if (LastValue.Equals(ConfigPair.Value)) 
        {
            continue;
        }

        // Save last config.
        OriginalClientConfigs.Add(ConfigPair.Key, LastValue);

        // Override the value.
        GConfig->SetString(*ClientSectionPath, *ConfigPair.Key, *ConfigPair.Value, GEngineIni);

        // Log the new value.
        FString ChangedValue;
        GConfig->GetString(*ClientSectionPath, *ConfigPair.Key, ChangedValue, GEngineIni);
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(
            Log,
            TEXT("Client SDK config %s is overridden from %s to %s"),
            *ConfigPair.Key,
            *LastValue,
            *ChangedValue);
    }

    // Try to override sdk config (server).
    for (auto& ConfigPair : ServerConfigs)
    {
        // Skip if the current value is the same.
        FString LastValue;
        GConfig->GetString(*ServerSectionPath, *ConfigPair.Key, LastValue, GEngineIni);
        if (LastValue.Equals(ConfigPair.Value))
        {
            continue;
        }

        // Save last config.
        OriginalServerConfigs.Add(ConfigPair.Key, LastValue);

        // Override the value.
        GConfig->SetString(*ServerSectionPath, *ConfigPair.Key, *ConfigPair.Value, GEngineIni);

        // Log the new value.
        FString ChangedValue;
        GConfig->GetString(*ServerSectionPath, *ConfigPair.Key, ChangedValue, GEngineIni);
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(
            Log,
            TEXT("Server SDK config %s is overridden from %s to %s"),
            *ConfigPair.Key,
            *LastValue,
            *ChangedValue);
    }

    // Abort if no configs to be overridden.
    if (OriginalClientConfigs.IsEmpty() && OriginalServerConfigs.IsEmpty()) 
    {
        return;
    }

    /* Overridden SDK config is placed at the default environment.
     * Thus, update the environment to use the default environment.*/
    IAccelByteUe4SdkModuleInterface::Get().SetEnvironment(ESettingsEnvironment::Default);

    /* Reset the original config values.
     * This way, the config file still remain original. */
    for (auto& OriginalConfigPair : OriginalClientConfigs)
    {
        GConfig->SetString(*ClientSectionPath, *OriginalConfigPair.Key, *OriginalConfigPair.Value, GEngineIni);
    }
    for (auto& OriginalConfigPair : OriginalServerConfigs)
    {
        GConfig->SetString(*ServerSectionPath, *OriginalConfigPair.Key, *OriginalConfigPair.Value, GEngineIni);
    }
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
    DedicatedServerVersionOverride = AccelByteWarsUtility::GetGameVersion();
    UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("DS version is overridden to: %s"), *DedicatedServerVersionOverride);
}

FString UTutorialModuleOnlineUtility::GetDedicatedServerVersionOverride()
{
    return GetClientVersionOverride().IsEmpty() ? DedicatedServerVersionOverride : GetClientVersionOverride();
}

TMap<FString, FString> UTutorialModuleOnlineUtility::CheckForSDKConfigOverride(const bool bIsServer)
{
    const FString ClientPrefix = TEXT("Client_");
    const FString ServerPrefix = TEXT("Server_");

    const FString ClientIdConfigKey = TEXT("ClientId");
    const FString ClientSecretConfigKey = TEXT("ClientSecret");
    const FString NamespaceConfigKey = TEXT("Namespace");
    const FString PublisherNamespaceConfigKey = TEXT("PublisherNamespace");
    const FString BaseURLConfigKey = TEXT("BaseUrl");
    const FString RedirectURIConfigKey = TEXT("RedirectURI");

    // These are supported SDK config keys to be overridden.
    const TArray<FString> ConfigKeys =
    {
        ClientIdConfigKey,
        ClientSecretConfigKey,
        NamespaceConfigKey,
        PublisherNamespaceConfigKey,
        BaseURLConfigKey,
        RedirectURIConfigKey
    };

    // These shared SDK config keys apply to both the game client and server.
    const TArray<FString> SharedConfigKeys =
    {
        NamespaceConfigKey,
        PublisherNamespaceConfigKey,
        BaseURLConfigKey
    };

    // Helper to store what SDK configs should be overridden.
    TMap<FString, FString> SDKConfigs;

    // Check launch param for the SDK config.
    const FString CmdArgs = FCommandLine::Get();
    const FString ConfigPrefix = bIsServer ? ServerPrefix : ClientPrefix;
    for (const FString& Config : ConfigKeys)
    {
        FString CmdValue = TEXT("");

        // Construct launch param to check.
        FString CmdStr = FString::Printf(TEXT("-%s%s="), *ConfigPrefix, *Config);
        if (SharedConfigKeys.Contains(Config))
        {
            /* The shared SDK config supports using client/server prefixes or without any prefixes.
             * The value changed by shared SDK config will affect both client and server SDK config value.
             * Example: -BaseUrl=test -Client_BaseUrl=test -Server_BaseUrl=test,
             * each of these commands will perform the same thing: to change the value of client and server Base Url to "test".*/

            // Construct shared SDK config launch param without any prefix.
            CmdStr = FString::Printf(TEXT("-%s="), *Config);

            // If the shared SDK config launch param without prefix is not found, fallback to add client prefix.
            if (!CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase)) 
            {
                CmdStr = FString::Printf(TEXT("-%s%s="), *ClientPrefix, *Config);
            }
            // If the shared SDK config launch param with client prefix is not found, fallback to add server prefix.
            if (!CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
            {
                CmdStr = FString::Printf(TEXT("-%s%s="), *ServerPrefix, *Config);
            }
        }

        // Check for launch param.
        if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
        {
            FParse::Value(*CmdArgs, *CmdStr, CmdValue);
        }
        else
        {
            continue;
        }

        // Abort if the SDK config to override value is empty.
        if (CmdValue.IsEmpty())
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(
                Warning,
                TEXT("Unable to override %s SDK config %s using launch param. Empty or invalid value."),
                bIsServer ? TEXT("Server") : TEXT("Client"),
                *Config);
            continue;
        }

        // Collect SDK config value to override.
        SDKConfigs.Add(Config, CmdValue);

        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(
            Log,
            TEXT("%s SDK config %s is overridden by launch param to %s"),
            bIsServer ? TEXT("Server") : TEXT("Client"),
            *Config,
            *CmdValue);
    }

    // Return SDK config to override.
    return SDKConfigs;
}

void UTutorialModuleOnlineUtility::CheckForEnvironmentConfigOverride()
{
    // Reset to default environment first.
    IAccelByteUe4SdkModuleInterface::Get().SetEnvironment(ESettingsEnvironment::Default);

    // Check for environment override.
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

void UTutorialModuleOnlineUtility::CheckForClientVersionOverride()
{
    ClientVersionOverride = TEXT("");

    const FString CmdArgs = FCommandLine::Get();
    const FString CmdStr = FString("-OverrideClientVersion=");
    if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
    {
        FString CmdValue = TEXT("");
        FParse::Value(*CmdArgs, *CmdStr, CmdValue);
        if (!CmdValue.IsEmpty())
        {
            ClientVersionOverride = CmdValue;
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Launch param sets the Client Version to %s."), *ClientVersionOverride);
        }
    }
}

void UTutorialModuleOnlineUtility::CheckForGameVersionOverride()
{
    GameVersionOverride = TEXT("");

    const FString CmdArgs = FCommandLine::Get();
    const FString CmdStr = FString("-OverrideGameVersion=");
    if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
    {
        FString CmdValue = TEXT("");
        FParse::Value(*CmdArgs, *CmdStr, CmdValue);
        if (!CmdValue.IsEmpty())
        {
            GameVersionOverride = CmdValue;
            AccelByteWarsUtility::SetGameVersion(GameVersionOverride);
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Launch param sets the Game Version to %s."), *GameVersionOverride);
        }
    }
}

void UTutorialModuleOnlineUtility::CheckSessionTemplateAndMatchPoolOverride()
{
    const TMap<FString, FString*> TemplateToOverrides
    {
        {TEXT("Dummy Session Template"), &DummySessionTemplateOverride},
        {TEXT("Party Session Template"), &PartySessionTemplateOverride},
        {TEXT("Match Session Template DS"), &MatchSessionTemplateDSOverride},
        {TEXT("Match Session Template P2P"), &MatchSessionTemplateP2POverride},
        {TEXT("Match Pool DS"), &MatchPoolDSOverride},
        {TEXT("Match Pool P2P"), &MatchPoolP2POverride},
    };

    // Check template value override.
    const FString CmdArgs = FCommandLine::Get();
    FString CmdStr = TEXT(""), CmdValue = TEXT("");
    for (const TPair<FString, FString*>& Template : TemplateToOverrides)
    {
        CmdValue = TEXT("");
        *Template.Value = TEXT("");

        CmdStr = FString::Printf(TEXT("-Override%s="), *Template.Key.Replace(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase));
        if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
        {
            CmdValue = TEXT("");
            FParse::Value(*CmdArgs, *CmdStr, CmdValue);

            if (!CmdValue.IsEmpty())
            {
                *Template.Value = CmdValue;
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Launch param set the override %s to %s."), *Template.Key, **Template.Value);
            }
        }
    }
}

void UTutorialModuleOnlineUtility::ExecutePredefinedServiceValidator(const EServicePredefinedValidator ValidatorType, const TDelegate<void(const bool /*bIsValid*/)>& OnComplete, const UObject* Context)
{
    if (!Context) 
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to execute predefined service validator. Context object is null."));
        OnComplete.ExecuteIfBound(false);
        return;
    }

    switch (ValidatorType)
    {
        case EServicePredefinedValidator::IS_REQUIRED_AMS_ACCOUNT:
        {
            AccelByte::FRegistry::AMS.GetAccount(
                AccelByte::THandler<FAccelByteModelsAMSGetAccountResponse>::CreateWeakLambda(this, [OnComplete](const FAccelByteModelsAMSGetAccountResponse& Result)
                {
                    UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Success to get AMS account configuration. AMS account id: %s"), *(!Result.Id.IsEmpty() ? Result.Id : FString("empty")));
                    OnComplete.ExecuteIfBound(!Result.Id.IsEmpty());
                }),
                FErrorHandler::CreateWeakLambda(this, [OnComplete](int32 ErrorCode, const FString& ErrorMessage)
                {
                    UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to get AMS account configuration. Error: %s"), *ErrorMessage);
                    OnComplete.ExecuteIfBound(false);
                }
            ));
            break;
        }
        case EServicePredefinedValidator::IS_VALID_CONFIG_VERSION:
        {
            const FString GameVersion = GetServicePredefinedArgument(EServicePredifinedArgument::GAME_VERSION);

            AccelByte::FRegistry::Configurations.Get(
                FString("bytewars_config_version"),
                THandler<FAccelByteModelsConfiguration>::CreateWeakLambda(this, [GameVersion, OnComplete](const FAccelByteModelsConfiguration& Result)
                {
                    if (Result.Value.IsEmpty()) 
                    {
                        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Failed to get config version from backend. Config version value is empty."));
                        return;
                    }

                    UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Success to get config version from backend. Config version value: %s, Game client version: %s"), *Result.Value, *GameVersion);
                    OnComplete.ExecuteIfBound(Result.Value.Equals(GameVersion));
                }),
                FErrorHandler::CreateWeakLambda(this, [OnComplete](int32 ErrorCode, const FString& ErrorMessage)
                {
                    UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to get config version from backend. Error: %s"), *ErrorMessage);
                    OnComplete.ExecuteIfBound(false);
                })
            );
            break;
        }
        case EServicePredefinedValidator::IS_ONLINE_SESSION:
        {
            const FString SessionId = GetServicePredefinedArgument(EServicePredifinedArgument::GAME_SESSION_ID);
            OnComplete.ExecuteIfBound(!SessionId.IsEmpty());
            break;
        }
        case EServicePredefinedValidator::IS_LOCAL_NETWORK:
        {
            const UWorld* World = Context->GetWorld();
            if (!World)
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to check local network type. World is not valid"));
                OnComplete.ExecuteIfBound(false);
                return;
            }

            if (const AAccelByteWarsGameState* GameState = Cast<AAccelByteWarsGameState>(World->GetGameState()))
            {
                OnComplete.ExecuteIfBound(GameState->GameSetup.NetworkType == EGameModeNetworkType::LOCAL);
            }
            else
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to check local network type. Game State is not valid"));
                OnComplete.ExecuteIfBound(false);
            }
            break;
        }
        case EServicePredefinedValidator::IS_P2P_NETWORK:
        {
            const UWorld* World = Context->GetWorld();
            if (!World) 
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to check P2P network type. World is not valid"));
                OnComplete.ExecuteIfBound(false);
                return;
            }

            if (const AAccelByteWarsGameState* GameState = Cast<AAccelByteWarsGameState>(World->GetGameState())) 
            {
                OnComplete.ExecuteIfBound(GameState->GameSetup.NetworkType == EGameModeNetworkType::P2P);
            }
            else 
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to check P2P network type. Game State is not valid"));
                OnComplete.ExecuteIfBound(false);
            }
            break;
        }
        case EServicePredefinedValidator::IS_DS_NETWORK:
        {
            const UWorld* World = Context->GetWorld();
            if (!World)
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to check DS network type. World is not valid"));
                OnComplete.ExecuteIfBound(false);
                return;
            }

            if (const AAccelByteWarsGameState* GameState = Cast<AAccelByteWarsGameState>(World->GetGameState()))
            {
                OnComplete.ExecuteIfBound(GameState->GameSetup.NetworkType == EGameModeNetworkType::DS);
            }
            else
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to check DS network type. Game State is not valid"));
                OnComplete.ExecuteIfBound(false);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void UTutorialModuleOnlineUtility::ExecutePredefinedServiceForFTUE(FFTUEDialogueModel* Dialogue, const FOnFTUEDialogueValidationComplete& OnComplete, const UObject* Context)
{
    if (!Dialogue)
    {
        return;
    }

    const EServicePredefinedValidator ValidatorType = Dialogue->Validator.ValidatorType;
    Dialogue->OnCustomValidationDelegate.Unbind();

    ExecutePredefinedServiceValidator(
        ValidatorType,
        TDelegate<void(const bool)>::CreateWeakLambda(this, [Dialogue, OnComplete](const bool bIsValid)
        {
            if (Dialogue)
            {
                OnComplete.ExecuteIfBound(Dialogue, !Dialogue->Validator.bNegateValidator ? bIsValid : !bIsValid);
            }
        }),
        Context
    );
}

void UTutorialModuleOnlineUtility::ExecutePredefinedServiceForWidgetValidator(FWidgetValidator* WidgetValidator, const UObject* Context)
{
    if (!WidgetValidator)
    {
        return;
    }

    const EServicePredefinedValidator ValidatorType = WidgetValidator->Validator.ValidatorType;
    WidgetValidator->OnValidatorExecutedDelegate.Unbind();

    ExecutePredefinedServiceValidator(
        ValidatorType,
        TDelegate<void(const bool)>::CreateWeakLambda(this, [WidgetValidator](const bool bIsValid)
        {
            if (WidgetValidator)
            {
                WidgetValidator->FinalizeValidator(!WidgetValidator->Validator.bNegateValidator ? bIsValid : !bIsValid);
            }
        }),
        Context
    );
}

FString UTutorialModuleOnlineUtility::GetServicePredefinedArgument(const EServicePredifinedArgument Keyword)
{
    FString Result = TEXT("");

    switch(Keyword) 
    {
        case EServicePredifinedArgument::PLAYER_ID:
        {
            Result = CurrentPlayerUserIdStr;
            break;
        }
        case EServicePredifinedArgument::PLAYER_DISPLAY_NAME:
        {
            Result = CurrentPlayerDisplayName;
            break;
        }
        case EServicePredifinedArgument::GAME_SESSION_ID:
        {
            if (FNamedOnlineSession* Session = GetOnlineSession(NAME_GameSession, this))
            {
                Result = Session->GetSessionIdStr();
            }
            break;
        }
        case EServicePredifinedArgument::PARTY_SESSION_ID:
        {
            if (FNamedOnlineSession* Session = GetOnlineSession(NAME_PartySession, this))
            {
                Result = Session->GetSessionIdStr();
            }
            break;
        }
        case EServicePredifinedArgument::DEDICATED_SERVER_ID:
        {
            Result = GetDedicatedServer(this).Server.Pod_name;
            break;
        }
        case EServicePredifinedArgument::ENV_BASE_URL:
        {
            const FString ClientBaseURL = AccelByte::FRegistry::Settings.BaseUrl;
            const FString ServerBaseURL = AccelByte::FRegistry::ServerSettings.BaseUrl;

            /* Since the environment URL config should be the same between client and server, try to get it from client first.
             * If it is empty, then get it from server config. */
            Result = !ClientBaseURL.IsEmpty() ? ClientBaseURL : ServerBaseURL;
            break;
        }
        case EServicePredifinedArgument::GAME_NAMESPACE:
        {
            const FString ClientGameNamespace = AccelByte::FRegistry::Settings.Namespace;
            const FString ServerGameNamespace = AccelByte::FRegistry::ServerSettings.Namespace;

            /* Since the game namespace config should be the same between client and server, try to get it from client first.
             * If it is empty, then get it from server config. */
            Result = !ClientGameNamespace.IsEmpty() ? ClientGameNamespace : ServerGameNamespace;
            break;
        }
        case EServicePredifinedArgument::ADMIN_PORTAL_URL:
        {
            const FString GameNamespace = GetServicePredefinedArgument(EServicePredifinedArgument::GAME_NAMESPACE);
            FString BaseURL = GetServicePredefinedArgument(EServicePredifinedArgument::ENV_BASE_URL);

            // For AGS Starter, the correct Admin Portal subdomain is "{studio_name}.{env}.gamingservices.accelbyte.io"
            if (IsUseAGSStarter()) 
            {
                BaseURL = BaseURL.Replace(*GameNamespace, *StudioNameAGSStarter);
            }

            Result = FString::Printf(TEXT("%s/admin/namespaces/%s"), *BaseURL, *GameNamespace);
            break;
        }
        case EServicePredifinedArgument::GAME_VERSION:
        {
            Result = AccelByteWarsUtility::GetGameVersion();
            break;
        }
        case EServicePredifinedArgument::PUBLISHED_STORE_ID:
        {
            Result = CurrentPublishedStoreId;
            break;
        }
        case EServicePredifinedArgument::TIME_UTC_NOW:
        {
            Result = FDateTime::UtcNow().ToIso8601();
            break;
        }
        default:
        {
            break;
        }
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

bool UTutorialModuleOnlineUtility::IsUseAGSStarter()
{
    return bUseAGSStarter;
}

bool UTutorialModuleOnlineUtility::IsUseVersionChecker()
{
    return bUseVersionChecker;
}

FString UTutorialModuleOnlineUtility::GetPrimaryLanguageSubtag()
{
    // Get the current language
    const FInternationalization& I18N = FInternationalization::Get();
    
    // Retrieve the two-letter ISO language name (primary language subtag)
    FString PrimaryLanguageSubtag = I18N.GetCurrentLanguage()->GetTwoLetterISOLanguageName();
    
    return PrimaryLanguageSubtag;
}

void UTutorialModuleOnlineUtility::IntializeAGSStaterIfValid()
{
    // Get game namespace from client or server creds.
    const FString GameNamespace = GetServicePredefinedArgument(EServicePredifinedArgument::GAME_NAMESPACE);
    const FString BaseURL = GetServicePredefinedArgument(EServicePredifinedArgument::ENV_BASE_URL);

    // AGS Starter is indicated by the game namespace id contained in the base URL.
    const FString AGSStarterBaseURLSuffix = TEXT("gamingservices.accelbyte.io");
    bUseAGSStarter = BaseURL.Contains(GameNamespace) && BaseURL.Contains(AGSStarterBaseURLSuffix);

    // Cache the AGS Starter studio name.
    if (bUseAGSStarter) 
    {
        // The studio name is the first word before the dash from the game namespace (e.g. {studioname-gamenamespace}).
        int32 SpacerIndex = GameNamespace.Find(TEXT("-"));
        StudioNameAGSStarter = GameNamespace.Left(SpacerIndex);
    }

    UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Is using AGS Starter: %s"), bUseAGSStarter ? TEXT("TRUE") : TEXT("FALSE"));
}

void UTutorialModuleOnlineUtility::CheckUseVersionChecker()
{
    // Check for launch parameter first.
    const FString CmdArgs = FCommandLine::Get();
    const FString CmdStr = FString("-UseVersionChecker=");
    bool bValidCmdValue = false;
    if (CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
    {
        FString CmdValue;
        FParse::Value(*CmdArgs, *CmdStr, CmdValue);
        if (!CmdValue.IsEmpty())
        {
            bUseVersionChecker = CmdValue.Equals(TEXT("TRUE"), ESearchCase::IgnoreCase);
            bValidCmdValue = true;

            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log,
                TEXT("Launch param sets the version checker to %s."),
                bUseVersionChecker ? TEXT("TRUE") : TEXT("FALSE"));
        }
    }

    // Check for DefaultEngine.ini
    if (!bValidCmdValue)
    {
        GConfig->GetBool(TEXT("AccelByteTutorialModules"), TEXT("bUseVersionChecker"), bUseVersionChecker, GEngineIni);

        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log,
            TEXT("DefaultEngine.ini sets the version checker to %s."),
            bUseVersionChecker ? TEXT("TRUE") : TEXT("FALSE"));
    }
}

void UTutorialModuleOnlineUtility::CacheGeneralInformation(const APlayerController* PC)
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!Subsystem)
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Cannot cache general information. Online Subsystem is not valid."));
        return;
    }

    const FOnlineIdentityAccelBytePtr IdentityInterface =
        StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
    if (!IdentityInterface)
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Cannot cache general information. Identity Interface is not valid."));
        return;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Cannot cache general information. Current logged-in player's is not valid."));
        return;
    }

    const FUniqueNetIdAccelByteUserPtr UserABId =
        StaticCastSharedPtr<const FUniqueNetIdAccelByteUser>(LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId());
    if (!UserABId)
    {
        UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Cannot cache general information. Current logged-in player's AccelByte user id is not valid."));
        return;
    }

    // Cache current logged-in player's AccelByte user id.
    CurrentPlayerUserIdStr = UserABId->GetAccelByteId();

    // Cache current logged-in player's display name.
    TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(UserABId.ToSharedRef().Get());
    if (UserAccount)
    {
        CurrentPlayerDisplayName = UserAccount->GetDisplayName();
    }
    if (CurrentPlayerDisplayName.IsEmpty())
    {
        CurrentPlayerDisplayName = GetUserDefaultDisplayName(CurrentPlayerUserIdStr);
    }

    // Cache current published store id.
    AccelByte::FRegistry::Item.GetListAllStores(
        THandler<TArray<FAccelByteModelsPlatformStore>>::CreateWeakLambda(this, [this](const TArray<FAccelByteModelsPlatformStore>& Result)
        {
            bool bHasPublishedStore = false;
            for (const FAccelByteModelsPlatformStore& Store : Result)
            {
                if (Store.Published)
                {
                    CurrentPublishedStoreId = Store.StoreId;
                    bHasPublishedStore = true;
                    break;
                }
            }

            if (bHasPublishedStore) 
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Log, TEXT("Success to cache published store id. Published store id: %s"), *CurrentPublishedStoreId);
            }
            else 
            {
                UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to cache published store id. Error: no store is published."));
            }
        }),
        FErrorHandler::CreateWeakLambda(this, [](int32 ErrorCode, const FString& ErrorMessage)
        {
            UE_LOG_TUTORIAL_MODULE_ONLINE_UTILITY(Warning, TEXT("Failed to cache published store id. Error %d: %s"), ErrorCode, *ErrorMessage);
        }
    ));
}
