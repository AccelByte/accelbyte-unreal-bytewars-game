// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "AuthEssentialsSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

// @@@SNIPSTART AuthEssentialsSubsystem.cpp-Initialize
// @@@MULTISNIP Interface {"selectedLines": ["1-2", "5-19", "22"]}
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
// @@@SNIPEND

void UAuthEssentialsSubsystem::Deinitialize()
{
    Super::Deinitialize();

    ClearAuthCredentials();
}

// @@@SNIPSTART AuthEssentialsSubsystem.cpp-Login
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
// @@@SNIPEND

// @@@SNIPSTART AuthEssentialsSubsystem.cpp-SetAuthCredentials
void UAuthEssentialsSubsystem::SetAuthCredentials(const EAccelByteLoginType& LoginMethod, const FString& Id, const FString& Token) 
{
    Credentials.Type = (LoginMethod == EAccelByteLoginType::None) ? TEXT("") : FAccelByteUtilities::GetUEnumValueAsString(LoginMethod);
    Credentials.Id = Id;
    Credentials.Token = Token;
}
// @@@SNIPEND

// @@@SNIPSTART AuthEssentialsSubsystem.cpp-ClearAuthCredentials
void UAuthEssentialsSubsystem::ClearAuthCredentials()
{
    Credentials.Type = TEXT("");
    Credentials.Id = TEXT("");
    Credentials.Token = TEXT("");
}
// @@@SNIPEND

TSharedPtr<FUserOnlineAccountAccelByte> UAuthEssentialsSubsystem::GetLoggedInUserOnlineAccount(const int LocalUserIndex) const
{
    /**
     * Use FUserOnlineAccount instead of FAccelByteUserInfo because it has more info.
     * FAccelByteUserInfo has LinkedPlatforms info
     */

    if (!IdentityInterface)
    {
        return nullptr;
    }

    // Get local user
    const APlayerController* PlayerController = UGameplayStatics::GetPlayerControllerFromID(this, LocalUserIndex);
    if (!(PlayerController && PlayerController->IsLocalPlayerController()))
    {
        return nullptr;
    }

    // Get ID
    const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    const FUniqueNetIdPtr UserId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
    if (!UserId)
    {
        return nullptr;
    }

    // Get OnlineAccount
    const TSharedPtr<FUserOnlineAccount> UserAccount = IdentityInterface->GetUserAccount(UserId.ToSharedRef().Get());
    if (!UserAccount)
    {
        return nullptr;
    }
    const TSharedPtr<FUserOnlineAccountAccelByte> AbUserAccount = StaticCastSharedPtr<FUserOnlineAccountAccelByte>(UserAccount);

    return AbUserAccount;
}

// @@@SNIPSTART AuthEssentialsSubsystem.cpp-OnLoginComplete
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
// @@@SNIPEND

TArray<UTutorialModuleSubsystem::FCheatCommandEntry> UAuthEssentialsSubsystem::GetCheatCommandEntries()
{
    TArray<FCheatCommandEntry> OutArray = {};

    // Get self user info
    OutArray.Add(FCheatCommandEntry(
        *CommandMyUserInfo,
        TEXT("Show logged in user info. Optional param: local user index"),
        FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::DisplayMyUserInfo)));

    return OutArray;
}

void UAuthEssentialsSubsystem::DisplayMyUserInfo(const TArray<FString>& Args) const
{
    int LocalUserIndex = 0;

    if (Args.Num() >= 1)
    {
        LocalUserIndex = FCString::Atoi(*Args[0]);
    }

    const TSharedPtr<FUserOnlineAccountAccelByte> OnlineAccount = GetLoggedInUserOnlineAccount(LocalUserIndex);
    if (!OnlineAccount.IsValid())
    {
        return;
    }
    const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const FUniqueNetIdAccelByteUser>(
        OnlineAccount->GetUserId());

    // Construct info
    FString AvatarURL = TEXT("");
    OnlineAccount->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);
    const FString OutString = FString::Printf(
        TEXT(
            "%sAB ID: %s%sPlatform type: %s%sPlatform ID: %s%sReal name: %s%sDisplay name: %s%sPublic Code: %s%sPlatform User ID: %s%sUser country: %s%sAccess token: %s%sSimultaneous platform ID: %s%sSimultaneous platform user ID: %s%s Connected to lobby: %s%s Connected to chat: %s%sAvatar URL: %s"),
        LINE_TERMINATOR,
        *UserABId->GetAccelByteId(),
        LINE_TERMINATOR,
        *UserABId->GetPlatformType(),
        LINE_TERMINATOR,
        *UserABId->GetPlatformId(),
        LINE_TERMINATOR,
        *OnlineAccount->GetRealName(),
        LINE_TERMINATOR,
        *OnlineAccount->GetDisplayName(),
        LINE_TERMINATOR,
        *OnlineAccount->GetPublicCode(),
        LINE_TERMINATOR,
        *OnlineAccount->GetPlatformUserId(),
        LINE_TERMINATOR,
        *OnlineAccount->GetUserCountry(),
        LINE_TERMINATOR,
        *OnlineAccount->GetAccessToken(),
        LINE_TERMINATOR,
        *OnlineAccount->GetSimultaneousPlatformID(),
        LINE_TERMINATOR,
        *OnlineAccount->GetSimultaneousPlatformUserID(),
        LINE_TERMINATOR,
        *FString(OnlineAccount->IsConnectedToLobby() ? TEXT("TRUE") : TEXT("FALSE")),
        LINE_TERMINATOR,
        *FString(OnlineAccount->IsConnectedToChat() ? TEXT("TRUE") : TEXT("FALSE")),
        LINE_TERMINATOR,
        *AvatarURL);

    GetWorld()->GetGameViewport()->ViewportConsole->OutputText(OutString);
}
