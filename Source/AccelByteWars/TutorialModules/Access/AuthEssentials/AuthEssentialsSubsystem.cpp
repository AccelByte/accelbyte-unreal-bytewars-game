// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AuthEssentialsSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

void UAuthEssentialsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
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

    Subsystem->SetLanguage(UTutorialModuleOnlineUtility::GetPrimaryLanguageSubtag());
}

void UAuthEssentialsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    ClearAuthCredentials();
}

void UAuthEssentialsSubsystem::Login(const APlayerController* PC, const FAuthOnLoginCompleteDelegate& OnLoginComplete)
{
    if (!ensure(IdentityInterface.IsValid()))
    {
        FString Message = TEXT("Cannot login. Identiy interface is not valid.");
        UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("%s"), *Message);
        OnLoginComplete.ExecuteIfBound(false, *Message);
        return;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    ensure(LocalPlayer != nullptr);
    int32 LocalUserNum = LocalPlayer->GetControllerId();
    
    IdentityInterface->AddOnLoginCompleteDelegate_Handle(LocalUserNum, FOnLoginCompleteDelegate::CreateUObject(this, &UAuthEssentialsSubsystem::OnLoginComplete, OnLoginComplete));
    IdentityInterface->Login(LocalUserNum, Credentials);

    /*
     * Logout On Game Exit
     * Workaround for the lobby not properly disconnect upon closing PIE game.
     */
    if (UAccelByteWarsGameInstance* ByteWarsGameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()); ensure(ByteWarsGameInstance))
    {
        ByteWarsGameInstance->OnGameInstanceShutdownDelegate.AddWeakLambda(this, [this, LocalUserNum]()
        {
            IdentityInterface->Logout(LocalUserNum);

            UE_LOG_AUTH_ESSENTIALS(Warning, TEXT("Logging out local player %d"), LocalUserNum);
        });
    }
}

void UAuthEssentialsSubsystem::SetAuthCredentials(const EAccelByteLoginType& LoginMethod, const FString& Id, const FString& Token) 
{
    Credentials.Type = (LoginMethod == EAccelByteLoginType::None) ? TEXT("") : FAccelByteUtilities::GetUEnumValueAsString(LoginMethod);
    Credentials.Id = Id;
    Credentials.Token = Token;
}

void UAuthEssentialsSubsystem::ClearAuthCredentials()
{
    Credentials.Type = TEXT("");
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");
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