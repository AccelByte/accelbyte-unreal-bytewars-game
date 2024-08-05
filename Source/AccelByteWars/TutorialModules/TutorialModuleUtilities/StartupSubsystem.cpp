// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "StartupSubsystem.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "UI/StartupWidget.h"
#include "OnlineSubsystemUtils.h"
#include "Access/AuthEssentials/AuthEssentialsModels.h"
#include "Access/AuthEssentials/UI/LoginWidget.h"
#include "Access/AuthEssentials/UI/LoginWidget_Starter.h"

void UStartupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializePlatformLogin();
}

void UStartupSubsystem::Deinitialize()
{
    Super::Deinitialize();

    DeinitializePlatformLogin();
}

#pragma region Login with Platform Only

void UStartupSubsystem::InitializePlatformLogin()
{
    // Abort if the game runs as dedicated server.
    if (IsRunningDedicatedServer())
    {
        UE_LOG_STARTUP(Log, TEXT("Abort to initialize platform login. The game is running as dedicated server."));
        return;
    }

    // Abort if no platform is activated.
    PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
    if (!PlatformSubsystem) 
    {
        UE_LOG_STARTUP(Log, TEXT("Abort to initialize platform login. No platform subsystem found."));
        return;
    }

    FOnlineSubsystemAccelByte* Subsystem = static_cast<FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
    if (!ensure(Subsystem))
    {
        UE_LOG_STARTUP(Warning, TEXT("Failed to initialize platform login. "
            "The online subsystem is invalid. "
            "Please make sure OnlineSubsystemAccelByte is enabled and "
            "DefaultPlatformService under[OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte Identity Interface and make sure it's valid.
    AccelByteIdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
    if (!ensure(AccelByteIdentityInterface.IsValid()))
    {
        UE_LOG_STARTUP(Warning, TEXT("Failed to initialize platform login. The Accelbyte identiy interface is not valid."));
        return;
    }

    PlatformIdentityInterface = PlatformSubsystem->GetIdentityInterface();
    if (!ensure(PlatformIdentityInterface.IsValid()))
    {
        UE_LOG_STARTUP(Warning, TEXT("Failed to initialize platform login. The platform identiy interface is not valid."));
        return;
    }

    PlatformType = FOnlineSubsystemAccelByteUtils::GetAccelByteLoginTypeFromNativeSubsystem(PlatformSubsystem->GetSubsystemName());

    CheckAutoUseTokenForABLogin();
    if (!IsAutoUseTokenForABLogin()) 
    {
        OverrideSinglePlatformAuthButtonAction();
    }
}

void UStartupSubsystem::DeinitializePlatformLogin()
{
    PlatformType = EAccelByteLoginType::None;
}

void UStartupSubsystem::CheckAutoUseTokenForABLogin()
{
    bAutoUseTokenForABLogin = true;

    const FString CmdArgs = FCommandLine::Get();
    const FString CmdStr = FString("-AutoUseTokenForABLogin=");

    if (!CmdArgs.Contains(CmdStr, ESearchCase::IgnoreCase))
    {
        return;
    }

    FString CmdValue;
    FParse::Value(*CmdArgs, *CmdStr, CmdValue);
    if (CmdValue.Equals(TEXT("FALSE"), ESearchCase::IgnoreCase))
    {
        bAutoUseTokenForABLogin = false;
    }
}

void UStartupSubsystem::OverrideSinglePlatformAuthButtonAction()
{
    UTutorialModuleDataAsset* AuthEssentialsDataAsset = 
        UTutorialModuleUtility::GetTutorialModuleDataAsset(FPrimaryAssetId("TutorialModule:AUTHESSENTIALS"), this);
    if (!ensure(AuthEssentialsDataAsset))
    {
        UE_LOG_STARTUP(Warning, TEXT(
            "Failed to override single platform auth login button event to login to AccelByte with platform credentials. "
            "The Auth Essentials module is not valid nor active."));
        return;
    }

    TSubclassOf<UAccelByteWarsActivatableWidget> LoginWidgetClass = AuthEssentialsDataAsset->GetTutorialModuleUIClass();
    if (!ensure(LoginWidgetClass))
    {
        UE_LOG_STARTUP(Warning, TEXT(
            "Failed to override single platform auth login button event to login to AccelByte with platform credentials. "
            "The login widget class is not valid."));
        return;
    }

    if (!ensure(LoginWidgetClass.GetDefaultObject()))
    {
        UE_LOG_STARTUP(Warning, TEXT(
            "Failed to override single platform auth login button event to login to AccelByte with platform credentials. "
            "The login widget class default object is not valid."));
        return;
    }

    // Override the single auth platform button action to login to AccelByte with platform credentials.
    LoginWidgetClass.GetDefaultObject()->OnActivated().AddWeakLambda(this, [this]()
        {
            const TArray<UUserWidget*> FoundWidgets =
                AccelByteWarsUtility::FindWidgetsOnTheScreen(
                    TEXT("Btn_LoginWithSinglePlatformAuth"),
                    UCommonButtonBase::StaticClass(),
                    false,
                    this);
            if (FoundWidgets.IsEmpty())
            {
                UE_LOG_STARTUP(Warning, TEXT(
                    "Failed to override single platform auth login button event to login to AccelByte with platform credentials. "
                    "The button is not found."));
                return;
            }

            UCommonButtonBase* SinglePlatformAuthLoginBtn = Cast<UCommonButtonBase>(FoundWidgets[0]);
            if (!SinglePlatformAuthLoginBtn)
            {
                UE_LOG_STARTUP(Warning, TEXT(
                    "Failed to override single platform auth login button event to login to AccelByte with platform credentials. "
                    "The button is not found."));
                return;
            }

            SinglePlatformAuthLoginBtn->OnClicked().Clear();
            SinglePlatformAuthLoginBtn->OnClicked().AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);
        }
    );
}

void UStartupSubsystem::OnLoginWithSinglePlatformAuthButtonClicked()
{
    UCommonActivatableWidget* ParentWidget = UAccelByteWarsBaseUI::GetActiveWidgetOfStack(EBaseUIStackType::Menu, this);
    if (!ensure(ParentWidget)) 
    {
        UE_LOG_STARTUP(Warning, TEXT("Failed to login to AccelByte with platform credentials. Parent widget is null."));
        return;
    }

    ULoginWidget* LoginWidget = Cast<ULoginWidget>(ParentWidget);
    ULoginWidget_Starter* LoginWidgetStarter = Cast<ULoginWidget_Starter>(ParentWidget);

    if (!ensure(LoginWidget) || !ensure(LoginWidgetStarter))
    {
        UE_LOG_STARTUP(Warning, TEXT("Failed to login to AccelByte with platform credentials. Login widget is null."));
        return;
    }

    if (LoginWidget)
    {
        LoginWidget->SetLoginState(ELoginState::LoggingIn);
        LoginWidget->OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);
        LoginPlatformCredsToAccelByte(
            LoginWidget->GetOwningPlayer(), 
            FOnLoginPlatformCompleteDelegate::CreateUObject(LoginWidget, &ULoginWidget::OnLoginComplete));
    }
    else if (LoginWidgetStarter)
    {
        LoginWidgetStarter->SetLoginState(ELoginState::LoggingIn);
        LoginWidgetStarter->OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);
        LoginPlatformCredsToAccelByte(
            LoginWidgetStarter->GetOwningPlayer(), 
            FOnLoginPlatformCompleteDelegate::CreateUObject(LoginWidgetStarter, &ULoginWidget_Starter::OnLoginComplete));
    }
}

void UStartupSubsystem::LoginPlatformOnly(const APlayerController* PC, const FOnLoginPlatformCompleteDelegate& OnLoginComplete)
{
    if (!PlatformSubsystem)
    {
        const FString ErrorMessage = TEXT("Failed to login with platform only. Platform subsystem is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    if (!PlatformIdentityInterface)
    {
        const FString ErrorMessage = TEXT("Failed to login with platform only. Platform identity interface is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    if (PlatformType == EAccelByteLoginType::None)
    {
        const FString ErrorMessage = TEXT("Failed to login with platform only. Unknown platform to login.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        const FString ErrorMessage = TEXT("Failed to login with platform only. Local player is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }
    
    const int32 LocalUserNum = LocalPlayer->GetControllerId();

    /* If not logged in yet, try open the target platform native login UI first.
     * This to allow the player to select the account to log in to.
     * After the account is selected, then retry to login with the selected account.
     * If no account selected or error, then always try to open the platform native login UI. */
    const ELoginStatus::Type PlatformLoginStatus = PlatformIdentityInterface->GetLoginStatus(LocalUserNum);
    if (!bPlatformAccountSelected && PlatformLoginStatus != ELoginStatus::LoggedIn)
    {
        const IOnlineExternalUIPtr PlatformExternalUI = PlatformSubsystem->GetExternalUIInterface();
        if (PlatformExternalUI.IsValid())
        {
            bool bPlatformLoginUIOpened = PlatformExternalUI->ShowLoginUI(
                LocalUserNum,
                true,
                false,
                FOnLoginUIClosedDelegate::CreateWeakLambda(this, [this, PC, OnLoginComplete](FUniqueNetIdPtr UniqueId, const int ControllerIndex, const FOnlineError& Error)
                {
                    // Set whether an account was selected successfully from the external platform UI.
                    // If the platform does not implement external platform UI, ignore the checker.
                    bPlatformAccountSelected = 
                        Error.ErrorCode.Equals(PLATFORM_LOGIN_UI_NOT_IMPLEMENTED_CODE, ESearchCase::IgnoreCase) ?
                        true : 
                        Error.bSucceeded && UniqueId.IsValid();

                    if (!bPlatformAccountSelected)
                    {
                        const FString ErrorMessage = FString::Printf(TEXT("Failed to select platform account to log in to. Error %s: %s"), *Error.ErrorCode, *Error.ErrorMessage.ToString());
                        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
                        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
                        return;
                    }

                    LoginPlatformOnly(PC, OnLoginComplete);
                }
            ));

            if (bPlatformLoginUIOpened)
            {
                UE_LOG_STARTUP(Log, TEXT("Platform login UI is opened, allowing the player to select which account to log in to."));
                return;
            }
        }
    }

    FOnlineAccountCredentials AccountCredentials;

    // Require complete credentials for EOS platform.
    if (PlatformSubsystem->GetSubsystemName().ToString().Equals(FAccelByteUtilities::GetUEnumValueAsString(EAccelByteLoginType::EOS), ESearchCase::IgnoreCase))
    {
        FAccelByteUtilities::GetValueFromCommandLineSwitch(AUTH_TYPE_PARAM, AccountCredentials.Type);
        FAccelByteUtilities::GetValueFromCommandLineSwitch(AUTH_LOGIN_PARAM, AccountCredentials.Id);
        FAccelByteUtilities::GetValueFromCommandLineSwitch(AUTH_PASSWORD_PARAM, AccountCredentials.Token);
    }

    PlatformIdentityInterface->AddOnLoginCompleteDelegate_Handle(
        LocalUserNum, 
        FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginPlatformOnlyComplete, OnLoginComplete));

    PlatformIdentityInterface->Login(LocalUserNum, AccountCredentials);
}

void UStartupSubsystem::LoginPlatformCredsToAccelByte(const APlayerController* PC, const FOnLoginPlatformCompleteDelegate& OnLoginComplete)
{
    if (PlatformCredentials.Type.IsEmpty() ||
        PlatformCredentials.Token.IsEmpty())
    {
        const FString ErrorMessage = TEXT("Failed to login to AccelByte with platform credentials. Platform credentials is empty. Try to login with platform only first.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    if (!AccelByteIdentityInterface)
    {
        const FString ErrorMessage = TEXT("Failed to login to AccelByte with platform credentials. AccelByte identity interface is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        const FString ErrorMessage = TEXT("Failed to login to AccelByte with platform credentials. Local player is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    const int32 LocalUserNum = LocalPlayer->GetControllerId();

    AccelByteIdentityInterface->AddOnLoginCompleteDelegate_Handle(
        LocalUserNum, 
        FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnLoginPlatformCredsToAccelByteComplete, OnLoginComplete));

    AccelByteIdentityInterface->Login(LocalUserNum, PlatformCredentials);
}

void UStartupSubsystem::OnLoginPlatformOnlyComplete(
    int32 LocalUserNum, 
    bool bSucceeded, 
    const FUniqueNetId& UserId, 
    const FString& LoginErrorMessage, 
    const FOnLoginPlatformCompleteDelegate OnLoginComplete)
{
    if (!PlatformSubsystem)
    {
        const FString ErrorMessage = TEXT("Cannot handle on-login with platform only complete event. Platform subsystem is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    if (!PlatformIdentityInterface)
    {
        const FString ErrorMessage = TEXT("Cannot handle on-login with platform only complete event. Platform identity interface is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    PlatformIdentityInterface->ClearOnLoginCompleteDelegates(LocalUserNum, this);

    if (!bSucceeded)
    {
        const FString ErrorMessage = FString::Printf(TEXT("Login with platform only failed. Error: %s"), *LoginErrorMessage);
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    FString PlatformTypeStr = FAccelByteUtilities::GetUEnumValueAsString(PlatformType);
    FString PlatformToken = FGenericPlatformHttp::UrlEncode(PlatformIdentityInterface->GetAuthToken(LocalUserNum));

    PlatformCredentials.Id = TEXT("");
    PlatformCredentials.Type = PlatformTypeStr;
    PlatformCredentials.Token = PlatformToken;

    UE_LOG_STARTUP(Log, TEXT("Login with platform only was successful. Platform name: %s, Platform token: %s"), *PlatformCredentials.Type, *PlatformCredentials.Token);

    // Bind on game exit event to log out from platform account.
    if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()); ensure(GameInstance))
    {
        GameInstance->OnGameInstanceShutdownDelegate.AddWeakLambda(this, [this, LocalUserNum]()
        {
            if (PlatformIdentityInterface) 
            {
                PlatformIdentityInterface->Logout(LocalUserNum);
            }
            
            UE_LOG_STARTUP(Warning, TEXT("Logging out local player %d from platform account."), LocalUserNum);
        });
    }

    OnLoginComplete.ExecuteIfBound(true, TEXT(""));
}

void UStartupSubsystem::OnLoginPlatformCredsToAccelByteComplete(
    int32 LocalUserNum, 
    bool bSucceeded, 
    const FUniqueNetId& UserId, 
    const FString& LoginErrorMessage, 
    const FOnLoginPlatformCompleteDelegate OnLoginComplete)
{
    if (!AccelByteIdentityInterface)
    {
        const FString ErrorMessage = TEXT("Cannot handle on-login to AccelByte with platform credentials complete event. AccelByte identity interface is null.");
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    AccelByteIdentityInterface->ClearOnLoginCompleteDelegates(LocalUserNum, this);

    if (!bSucceeded)
    {
        const FString ErrorMessage = FString::Printf(TEXT("Login to AccelByte with platform credentials failed. Error: %s"), *LoginErrorMessage);
        UE_LOG_STARTUP(Warning, TEXT("%s"), *ErrorMessage);
        OnLoginComplete.ExecuteIfBound(false, ErrorMessage);
        return;
    }

    UE_LOG_STARTUP(Log, TEXT("Login to AccelByte with platform credentials was successful."));

    // Bind on game exit event to log out from AccelByte account.
    if (UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance()); ensure(GameInstance))
    {
        GameInstance->OnGameInstanceShutdownDelegate.AddWeakLambda(this, [this, LocalUserNum]()
        {
            if (AccelByteIdentityInterface)
            {
                AccelByteIdentityInterface->Logout(LocalUserNum);
            }

            UE_LOG_STARTUP(Warning, TEXT("Logging out local player %d from AccelByte account."), LocalUserNum);
        });
    }

    OnLoginComplete.ExecuteIfBound(true, TEXT(""));
}

#pragma endregion