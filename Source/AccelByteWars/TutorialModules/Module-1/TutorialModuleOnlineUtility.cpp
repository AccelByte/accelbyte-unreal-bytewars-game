// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-1/TutorialModuleOnlineUtility.h"
#include "Core/Utilities/AccelByteWarsBlueprintFunctionLibrary.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"

#include "Core/AccelByteRegistry.h"
#include "AccelByteUe4SdkModule.h"

#include "TutorialModules/Module-2/AuthEssentialsModels.h"

DEFINE_LOG_CATEGORY(LogAccelByteWarsTutorialModuleOnlineUtility);

//UTutorialModuleOnlineUtility::CurrentPlayerUserIdStr = FString("");
//UTutorialModuleOnlineUtility::CurrentPlayerDisplayName = FString("");

UTutorialModuleOnlineUtility::UTutorialModuleOnlineUtility()
{
    CheckForEnvironmentConfigOverride();

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
            if (CurrentPlayerDisplayName.IsEmpty()) 
            {
                CurrentPlayerDisplayName = GetUserDefaultDisplayName(UserABId.ToSharedRef().Get());
            }
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
