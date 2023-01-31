// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Access/AuthEssentials/AuthEssentialsSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UAuthEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
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

void UAuthEssentialsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    ClearAuthCredentials(true);
}

void UAuthEssentialsSubsystem::Login(EAccelByteLoginType LoginMethod, const APlayerController* PC, const FAuthOnLoginCompleteDelegate& OnLoginComplete)
{
    if (!ensure(IdentityInterface.IsValid()))
    {
        FString Message = TEXT("Cannot login. Identiy interface is not valid.");
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("%s"), *Message);
        OnLoginComplete.ExecuteIfBound(false, *Message);
        return;
    }

    // Set login type.
    Credentials.Type = FAccelByteUtilities::GetUEnumValueAsString(LoginMethod);

    // Set login credentials based on login type.
    switch (LoginMethod)
    {
        case EAccelByteLoginType::DeviceId:
            // Login with device id doesn't requires id and token credentials.
            ClearAuthCredentials();
            break;
    }

    int32 LocalUserNum = GetLocalUserNum(PC);
    IdentityInterface->AddOnLoginCompleteDelegate_Handle(LocalUserNum, FOnLoginCompleteDelegate::CreateUObject(this, &UAuthEssentialsSubsystem::OnLoginComplete, OnLoginComplete));
    IdentityInterface->Login(LocalUserNum, Credentials);
}

void UAuthEssentialsSubsystem::Logout(const APlayerController* PC, const FAuthOnLogoutCompleteDelegate& OnLogoutComplete)
{
    if (!ensure(IdentityInterface.IsValid())) 
    {
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Cannot logout. Identiy interface is not valid."));
        OnLogoutComplete.ExecuteIfBound(false);
        return;
    }

    int32 LocalUserNum = GetLocalUserNum(PC);
    IdentityInterface->AddOnLogoutCompleteDelegate_Handle(LocalUserNum, FOnLogoutCompleteDelegate::CreateUObject(this, &UAuthEssentialsSubsystem::OnLogoutComplete, OnLogoutComplete));
    IdentityInterface->Logout(LocalUserNum);
}

bool UAuthEssentialsSubsystem::IsLoggedIn(const APlayerController* PC)
{
    if (!ensure(IdentityInterface.IsValid()))
    {
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Cannot get login status. Identiy interface is not valid."));
        return ELoginStatus::Type::NotLoggedIn;
    }

    ELoginStatus::Type Status = IdentityInterface->GetLoginStatus(GetLocalUserNum(PC));
    return Status == ELoginStatus::Type::LoggedIn;
}

void UAuthEssentialsSubsystem::SetAuthCredentials(const FString& Id, const FString& Token) 
{
    Credentials.Id = Id;
    Credentials.Token = Token;
}

void UAuthEssentialsSubsystem::ClearAuthCredentials(bool bAlsoResetType)
{
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");

    if (bAlsoResetType) 
    {
        Credentials.Type = TEXT("");
    }
}

void UAuthEssentialsSubsystem::OnLoginComplete(int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError, const FAuthOnLoginCompleteDelegate OnLoginComplete)
{
    if (bLoginWasSuccessful)
    {
        UE_LOG_AUTH_ESSENTIALS(Log, TEXT("Login user successful."));
    }
    else
    {
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Login user failed. Message: %s"), *LoginError);
    }

    IdentityInterface->ClearOnLoginCompleteDelegates(LocalUserNum, this);
    OnLoginComplete.ExecuteIfBound(bLoginWasSuccessful, LoginError);
}

void UAuthEssentialsSubsystem::OnLogoutComplete(int32 LocalUserNum, bool bLogoutWasSuccessful, const FAuthOnLogoutCompleteDelegate OnLogoutComplete)
{
    if (bLogoutWasSuccessful)
    {
        UE_LOG_AUTH_ESSENTIALS(Log, TEXT("Logout user is successful."));
    }
    else
    {
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Logout user failed."));
    }

    IdentityInterface->ClearOnLogoutCompleteDelegates(LocalUserNum, this);
    OnLogoutComplete.ExecuteIfBound(bLogoutWasSuccessful);
}

int32 UAuthEssentialsSubsystem::GetLocalUserNum(const APlayerController* PC)
{
    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);

    return LocalPlayer->GetControllerId();
}