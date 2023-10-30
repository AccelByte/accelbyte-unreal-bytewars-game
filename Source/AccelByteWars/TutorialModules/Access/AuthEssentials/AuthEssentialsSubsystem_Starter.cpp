// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AuthEssentialsSubsystem_Starter.h"
#include "OnlineSubsystemUtils.h"

void UAuthEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem)) 
    {
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte Identity Interface and make sure it's valid.
    IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
    if (!ensure(IdentityInterface.IsValid()))
    {
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Identiy interface is not valid."));
        return;
    }
}

void UAuthEssentialsSubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();

    ClearAuthCredentials();
}

void UAuthEssentialsSubsystem_Starter::SetAuthCredentials(const EAccelByteLoginType& LoginMethod, const FString& Id, const FString& Token)
{
    Credentials.Type = (LoginMethod == EAccelByteLoginType::None) ? TEXT("") : FAccelByteUtilities::GetUEnumValueAsString(LoginMethod);
    Credentials.Id = Id;
    Credentials.Token = Token;
}

void UAuthEssentialsSubsystem_Starter::ClearAuthCredentials()
{
    Credentials.Type = TEXT("");
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");
}

void UAuthEssentialsSubsystem_Starter::Login(const APlayerController* PC, const FAuthOnLoginCompleteDelegate_Starter& OnLoginComplete)
{
    // TODO: Implement login functionality here.
    UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Login functionality is not yet implemented."))
}