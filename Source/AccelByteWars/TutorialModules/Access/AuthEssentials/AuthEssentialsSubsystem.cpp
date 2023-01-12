// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Access/AuthEssentials/AuthEssentialsSubsystem.h"
#include "OnlineSubsystemUtils.h"

void UAuthEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get AccelByte Identity Interface from OSS.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    ensure(Subsystem != nullptr);
    IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
    ensure(IdentityInterface.IsValid());

    ClearAuthCredentials(true);
}

void UAuthEssentialsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    ClearAuthCredentials(true);
}

void UAuthEssentialsSubsystem::Login(EAccelByteLoginType LoginMethod, const APlayerController* PC, const FOnLoginComplete& OnLoginComplete)
{
    Credentials.Type = FAccelByteUtilities::GetUEnumValueAsString(LoginMethod);

    switch (LoginMethod)
    {
        case EAccelByteLoginType::DeviceId:
            // Login with device id doesn't requires id and token credentials.
            ClearAuthCredentials();
            break;
        case EAccelByteLoginType::Steam:
            // Steam is one of single auth platform. 
            // Single auth platform method doesn't requires id, token, and type credentials.
            ClearAuthCredentials(true);
            break;
    }

    int32 LocalUserNum = GetLocalUserNum(PC);
    IdentityInterface->AddOnLoginCompleteDelegate_Handle(LocalUserNum, FOnLoginCompleteDelegate::CreateUObject(this, &UAuthEssentialsSubsystem::OnLoginComplete, OnLoginComplete));
    IdentityInterface->Login(LocalUserNum, Credentials);
}

void UAuthEssentialsSubsystem::Logout(const APlayerController* PC, const FOnLogoutComplete& OnLogoutComplete)
{
    int32 LocalUserNum = GetLocalUserNum(PC);
    IdentityInterface->AddOnLogoutCompleteDelegate_Handle(LocalUserNum, FOnLogoutCompleteDelegate::CreateUObject(this, &UAuthEssentialsSubsystem::OnLogoutComplete, OnLogoutComplete));
    IdentityInterface->Logout(LocalUserNum);
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

bool UAuthEssentialsSubsystem::IsLoggedIn(const APlayerController* PC)
{
    ELoginStatus::Type Status = IdentityInterface->GetLoginStatus(GetLocalUserNum(PC));
    return Status == ELoginStatus::Type::LoggedIn;
}

int32 UAuthEssentialsSubsystem::GetLocalUserNum(const APlayerController* PC)
{
    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);

    return LocalPlayer->GetControllerId();
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

    IdentityInterface->ClearOnLoginCompleteDelegates(LocalUserNum, this);
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

    IdentityInterface->ClearOnLogoutCompleteDelegates(LocalUserNum, this);
    OnLogoutComplete.ExecuteIfBound(bLogoutWasSuccessful);
}