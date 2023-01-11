// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Access/AuthEssentials/AuthEssentialsSubsystem/AuthEssentialsSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UAuthEssentialsSubsystem::Login(EAccelByteLoginType LoginMethod, const APlayerController* PC, const FOnLoginComplete& OnLoginComplete)
{
    const FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface();
    ensure(IdentityInterface.IsValid());

    const ULocalPlayer* LocalUser = PC->GetLocalPlayer();
    ensure(LocalUser != nullptr);

    const int32 LocalUserNum = LocalUser->GetControllerId();

    Credentials.LoginType = LoginMethod;
    Credentials.Type = FAccelByteUtilities::GetUEnumValueAsString(LoginMethod);

    LoginCompleteDelegate = IdentityInterface->AddOnLoginCompleteDelegate_Handle(LocalUserNum, FOnLoginCompleteDelegate::CreateUObject(this, &UAuthEssentialsSubsystem::OnLoginComplete, OnLoginComplete));
    IdentityInterface->Login(LocalUserNum, Credentials);
}

void UAuthEssentialsSubsystem::Logout(const APlayerController* PC, const FOnLogoutComplete& OnLogoutComplete)
{
    const FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface();
    ensure(IdentityInterface.IsValid());

    const ULocalPlayer* LocalUser = PC->GetLocalPlayer();
    ensure(LocalUser != nullptr);

    const int32 LocalUserNum = LocalUser->GetControllerId();

    LogoutCompleteDelegate = IdentityInterface->AddOnLogoutCompleteDelegate_Handle(LocalUserNum, FOnLogoutCompleteDelegate::CreateUObject(this, &UAuthEssentialsSubsystem::OnLogoutComplete, OnLogoutComplete));
    IdentityInterface->Logout(LocalUserNum);
}

bool UAuthEssentialsSubsystem::IsLoggedIn(const APlayerController* PC)
{
    const FOnlineIdentityAccelBytePtr IdentityInterface = GetIdentityInterface();
    ensure(IdentityInterface.IsValid());

    const ULocalPlayer* LocalUser = PC->GetLocalPlayer();
    ensure(LocalUser != nullptr);

    const int32 LocalUserNum = LocalUser->GetControllerId();

    ELoginStatus::Type Status = IdentityInterface->GetLoginStatus(LocalUserNum);
    return Status == ELoginStatus::Type::LoggedIn;
}

void UAuthEssentialsSubsystem::SetAuthCredentials(const FString& Id, const FString& Token)
{
    Credentials.Id = Id;
    Credentials.Token = Token;
}

void UAuthEssentialsSubsystem::SetPlatformAuthCredentials(const APlayerController* PC)
{
    const IOnlineSubsystem* NativeSubsystem = IOnlineSubsystem::GetByPlatform();
    ensure(NativeSubsystem != nullptr);

    const IOnlineIdentityPtr NativeIdentityInterface = NativeSubsystem->GetIdentityInterface();
    ensure(NativeIdentityInterface.IsValid());

    const ULocalPlayer* LocalUser = PC->GetLocalPlayer();
    ensure(LocalUser != nullptr);

    const int32 LocalUserNum = LocalUser->GetControllerId();

    Credentials.Token = FGenericPlatformHttp::UrlEncode(NativeIdentityInterface->GetAuthToken(LocalUserNum));
}

FOnlineIdentityAccelBytePtr UAuthEssentialsSubsystem::GetIdentityInterface() const
{
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem != nullptr))
    {
        return nullptr;
    }

    return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

void UAuthEssentialsSubsystem::OnLoginComplete(int32 LocalUserNum, bool bLoginWasSuccessful, const FUniqueNetId& UserId, const FString& LoginError, const FOnLoginComplete OnLoginComplete)
{
    if (bLoginWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Login user is successful."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Login user failed. Message: %s"), *LoginError);
    }

    GetIdentityInterface()->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginCompleteDelegate);
    OnLoginComplete.ExecuteIfBound(bLoginWasSuccessful, LoginError);
}

void UAuthEssentialsSubsystem::OnLogoutComplete(int32 LocalUserNum, bool bLogoutWasSuccessful, const FOnLogoutComplete OnLogoutComplete)
{
    if (bLogoutWasSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Logout user is successful."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Logout user failed."));
    }

    GetIdentityInterface()->ClearOnLogoutCompleteDelegate_Handle(LocalUserNum, LogoutCompleteDelegate);
    OnLogoutComplete.ExecuteIfBound(bLogoutWasSuccessful);
}