// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "StartupSubsystem.h"
#include "Core/Utilities/AccelByteWarsUtility.h"
#include "Core/UI/AccelByteWarsBaseUI.h"
#include "UI/StartupWidget.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineUserInterface.h"
#include "StartupLog.h"
#include "Access/AuthEssentials/AuthEssentialsModels.h"
#include "Access/AuthEssentials/UI/LoginWidget.h"
#include "Access/AuthEssentials/UI/LoginWidget_Starter.h"
#include "Core/GameModes/AccelByteWarsInGameGameMode.h"
#include "Core/Player/AccelByteWarsPlayerController.h"
#include "Core/UI/MainMenu/MatchLobby/MatchLobbyWidget.h"

void UStartupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize online user interface
	const UWorld* World = GetWorld();
	ensure(World);
	UserInterface = Online::GetUserInterface(World);

	InitializePlatformLogin();

	if (!IsRunningDedicatedServer())
	{
		UMatchLobbyWidget::OnQueryTeamMembersInfoDelegate.AddUObject(this, &ThisClass::OnQueryTeamMembersInfo);
	}

#pragma region "Core game cheats"
	if (UGUICheatWidgetEntry* GUICheatEntryKillSelf = FTutorialModuleGeneratedWidget::GetGUICheatMetadataById(GUI_CHEAT_ENTRY_KILL_SELF))
	{
		GUICheatEntryKillSelf->OnClicked.AddDynamic(this, &ThisClass::KillSelf);
	}
	if (UGUICheatWidgetEntry* GUICheatEntryKillOthers = FTutorialModuleGeneratedWidget::GetGUICheatMetadataById(GUI_CHEAT_ENTRY_KILL_OTHERS))
	{
		GUICheatEntryKillOthers->OnClicked.AddDynamic(this, &ThisClass::KillOthers);
	}
#pragma endregion 
}

void UStartupSubsystem::Deinitialize()
{
	Super::Deinitialize();

	DeinitializePlatformLogin();

	if (!IsRunningDedicatedServer())
	{
		UMatchLobbyWidget::OnQueryTeamMembersInfoDelegate.RemoveAll(this);
	}
}

#pragma region "Common functions"
bool UStartupSubsystem::CompareAccelByteUniqueId(
	const FUniqueNetIdRepl& FirstUniqueNetId,
	const FUniqueNetIdRepl& SecondUniqueNetId) const
{
	if (!FirstUniqueNetId.IsValid() || !SecondUniqueNetId.IsValid())
	{
		return false;
	}

	// compare directly first
	if (FirstUniqueNetId == SecondUniqueNetId)
	{
		return true;
	}

	// if false, attempt to compare AB User Id first
	if (!FirstUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE) ||
		!SecondUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE))
	{
		return false;
	}
	
	const FUniqueNetIdAccelByteUserPtr FirstAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*FirstUniqueNetId);
	const FUniqueNetIdAccelByteUserPtr SecondAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*SecondUniqueNetId);

	if (!FirstAbUniqueNetId.IsValid() || !SecondAbUniqueNetId.IsValid())
	{
		return false;
	}

	const FString FirstAbUserId = FirstAbUniqueNetId->GetAccelByteId();
	const FString SecondAbUserId = SecondAbUniqueNetId->GetAccelByteId();

	return FirstAbUserId.Equals(SecondAbUserId);
}

bool UStartupSubsystem::CompareAccelByteUniqueId(
	const FUniqueNetIdRepl& FirstUniqueNetId,
	const FString& SecondAbUserId) const
{
	if (!FirstUniqueNetId.IsValid() || !FirstUniqueNetId->GetType().IsEqual(ACCELBYTE_USER_ID_TYPE))
	{
		return false;
	}

	const FUniqueNetIdAccelByteUserPtr FirstAbUniqueNetId = FUniqueNetIdAccelByteUser::TryCast(*FirstUniqueNetId);
	if (!FirstAbUniqueNetId.IsValid())
	{
		return false;
	}
	const FString FirstAbUserId = FirstAbUniqueNetId->GetAccelByteId();

	return FirstAbUserId.Equals(SecondAbUserId);
}

// @@@SNIPSTART StartupSubsystem.cpp-QueryUserInfo
void UStartupSubsystem::QueryUserInfo(
	const int32 LocalUserNum,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FOnQueryUsersInfoCompleteDelegate& OnComplete)
{
	// Abort if the user interface is invalid.
	if (!UserInterface)
	{
		UE_LOG_STARTUP(Warning, TEXT("User interface is invalid"))
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, OnComplete]()
		{
			OnComplete.ExecuteIfBound(
				FOnlineError::CreateError(
					TEXT(""),
					EOnlineErrorResult::RequestFailure,
					TEXT(""),
					FText::FromString(TEXT(""))),
				{});
		}));
		return;
	}

	// Call query right away since the it will check for cache first internally
	if (OnQueryUserInfoCompleteDelegateHandle.IsValid())
	{
		UserInterface->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
		OnQueryUserInfoCompleteDelegateHandle.Reset();
	}
	OnQueryUserInfoCompleteDelegateHandle = UserInterface->OnQueryUserInfoCompleteDelegates->AddWeakLambda(
		this, [OnComplete, this](
			int32 LocalUserNum,
			bool bSucceeded,
			const TArray<FUniqueNetIdRef>& UserIds,
			const FString& ErrorMessage)
		{
			OnQueryUserInfoComplete(LocalUserNum, bSucceeded, UserIds, ErrorMessage, OnComplete);
		});

	if (!UserInterface->QueryUserInfo(LocalUserNum, UserIds))
	{
		OnQueryUserInfoComplete(LocalUserNum, false, UserIds, TEXT(""), OnComplete);
	}
}
// @@@SNIPEND

// @@@SNIPSTART StartupSubsystem.cpp-OnQueryUserInfoComplete
void UStartupSubsystem::OnQueryUserInfoComplete(
	int32 LocalUserNum,
	bool bSucceeded,
	const TArray<FUniqueNetIdRef>& UserIds,
	const FString& ErrorMessage,
	const FOnQueryUsersInfoCompleteDelegate& OnComplete)
{
	// reset delegate handle
	UserInterface->OnQueryUserInfoCompleteDelegates->Remove(OnQueryUserInfoCompleteDelegateHandle);
	OnQueryUserInfoCompleteDelegateHandle.Reset();

	if (bSucceeded)
	{
		// Retrieve the result from cache.
		TArray<TSharedPtr<FUserOnlineAccountAccelByte>> OnlineUsers;
		for (const FUniqueNetIdRef& UserId : UserIds)
		{
			TSharedPtr<FOnlineUser> OnlineUser = UserInterface->GetUserInfo(0, UserId.Get());
			if (!OnlineUser.IsValid() || !OnlineUser->GetUserId()->IsValid())
			{
				continue;
			}
			OnlineUsers.Add(StaticCastSharedPtr<FUserOnlineAccountAccelByte>(OnlineUser));
		}

		OnComplete.ExecuteIfBound(FOnlineError::Success(), OnlineUsers);
	}
	else
	{
		OnComplete.ExecuteIfBound(
			FOnlineError::CreateError(
				TEXT(""),
				EOnlineErrorResult::RequestFailure,
				TEXT(""),
				FText::FromString(ErrorMessage)),
			{});
	}
}
// @@@SNIPEND
#pragma endregion 

#pragma region Login with Platform Only

void UStartupSubsystem::InitializePlatformLogin()
{
	// Abort if the game runs as dedicated server.
	if (IsRunningDedicatedServer())
	{
		UE_LOG_STARTUP(Log, TEXT("Abort to initialize platform login. The game is running as dedicated server."));
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
		UE_LOG_STARTUP(Warning, TEXT("Failed to initialize platform login. The Accelbyte Identity interface is not valid."));
		return;
	}

	// Abort if no platform is activated.
	PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
	if (!PlatformSubsystem) 
	{
		UE_LOG_STARTUP(Log, TEXT("Abort to initialize platform login. No platform subsystem found."));
		return;
	}

	PlatformIdentityInterface = PlatformSubsystem->GetIdentityInterface();
	if (!ensure(PlatformIdentityInterface.IsValid()))
	{
		UE_LOG_STARTUP(Warning, TEXT("Failed to initialize platform login. The platform Identity interface is not valid."));
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

	ULoginWidget* LoginWidget = StaticCast<ULoginWidget*>(ParentWidget);
	ULoginWidget_Starter* LoginWidgetStarter = StaticCast<ULoginWidget_Starter*>(ParentWidget);

	if (!ensure(LoginWidget) || !ensure(LoginWidgetStarter))
	{
		UE_LOG_STARTUP(Warning, TEXT("Failed to login to AccelByte with platform credentials. Login widget is null."));
		return;
	}

	if (LoginWidget)
	{
		LoginWidget->SetLoginState(ELoginState::LoggingIn);
		LoginWidget->OnRetryLoginDelegate.RemoveAll(this);
		LoginWidget->OnRetryLoginDelegate.AddUObject(this, &ThisClass::OnLoginWithSinglePlatformAuthButtonClicked);
		LoginPlatformCredsToAccelByte(
			LoginWidget->GetOwningPlayer(), 
			FOnLoginPlatformCompleteDelegate::CreateUObject(LoginWidget, &ULoginWidget::OnLoginComplete));
	}
	else if (LoginWidgetStarter)
	{
		LoginWidgetStarter->SetLoginState(ELoginState::LoggingIn);
		LoginWidgetStarter->OnRetryLoginDelegate.RemoveAll(this);
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

TArray<UTutorialModuleSubsystem::FCheatCommandEntry> UStartupSubsystem::GetCheatCommandEntries()
{
	TArray<FCheatCommandEntry> OutArray = {};

	// Retrieve and display user info given AB's ID
	OutArray.Add(FCheatCommandEntry(
		*CommandUserInfo,
		TEXT("Query user info by user ID. Requires user ID as the first param"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::DisplayUserInfo)));

	// Lobby connect disconnect
	OutArray.Add(FCheatCommandEntry(
		*CommandLobbyConnect,
		TEXT("Connect to Lobby websocket"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::LobbyConnect)));
	OutArray.Add(FCheatCommandEntry(
		*CommandLobbyDisconnect,
		TEXT("Disconnect from Lobby websocket"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &ThisClass::LobbyDisconnect)));

	return OutArray;
}

void UStartupSubsystem::DisplayUserInfo(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		return;
	}

	// Construct UniqueNetId
	FUniqueNetIdAccelByteUserRef UniqueNetId = FUniqueNetIdAccelByteUser::Create(Args[0]);

	QueryUserInfo(0, {UniqueNetId}, FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(
		this, [this](
			const FOnlineError& Error,
			const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
		{
			if (!Error.bSucceeded || UsersInfo.IsEmpty())
			{
				return;
			}

			const TSharedPtr<FUserOnlineAccountAccelByte> UserInfo = UsersInfo[0];
			if (!UserInfo.IsValid())
			{
				return;
			}
			const FUniqueNetIdAccelByteUserRef UserABId = StaticCastSharedRef<const
				FUniqueNetIdAccelByteUser>(UserInfo->GetUserId());

			// Construct info
			FString AvatarURL = TEXT("");
			UserInfo->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, AvatarURL);
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
				*UserInfo->GetRealName(),
				LINE_TERMINATOR,
				*UserInfo->GetDisplayName(),
				LINE_TERMINATOR,
				*UserInfo->GetPublicCode(),
				LINE_TERMINATOR,
				*UserInfo->GetPlatformUserId(),
				LINE_TERMINATOR,
				*UserInfo->GetUserCountry(),
				LINE_TERMINATOR,
				*UserInfo->GetAccessToken(),
				LINE_TERMINATOR,
				*UserInfo->GetSimultaneousPlatformID(),
				LINE_TERMINATOR,
				*UserInfo->GetSimultaneousPlatformUserID(),
				LINE_TERMINATOR,
				*FString(UserInfo->IsConnectedToLobby() ? TEXT("TRUE") : TEXT("FALSE")),
				LINE_TERMINATOR,
				*FString(UserInfo->IsConnectedToChat() ? TEXT("TRUE") : TEXT("FALSE")),
				LINE_TERMINATOR,
				*AvatarURL);

			GetWorld()->GetGameViewport()->ViewportConsole->OutputText(OutString);
		}));
}

void UStartupSubsystem::LobbyConnect(const TArray<FString>& Args)
{
	if (!AccelByteIdentityInterface)
	{
		InitializePlatformLogin();
	}

	int LoginUserNum = 0;
	if (Args.Num() >= 1)
	{
		LoginUserNum = FCString::Atoi(*Args[0]);
	}

	const AccelByte::FApiClientPtr ApiClient = AccelByteIdentityInterface->GetApiClient(LoginUserNum);
	if (!ApiClient) 
	{
		UE_LOG_STARTUP(Warning, TEXT("Cannot connect to lobby. AccelByte API Client is invalid."));
		return;
	}

	AccelByte::Api::LobbyPtr LobbyApi = ApiClient->GetLobbyApi().Pin();
	if (!LobbyApi) 
	{
		UE_LOG_STARTUP(Warning, TEXT("Cannot connect to lobby. Lobby API is invalid."));
		return;
	}

	LobbyApi->Connect();
}

void UStartupSubsystem::LobbyDisconnect(const TArray<FString>& Args)
{
	if (!AccelByteIdentityInterface)
	{
		InitializePlatformLogin();
	}

	int LoginUserNum = 0;
	if (Args.Num() >= 1)
	{
		LoginUserNum = FCString::Atoi(*Args[0]);
	}

	const AccelByte::FApiClientPtr ApiClient = AccelByteIdentityInterface->GetApiClient(LoginUserNum);
	if (!ApiClient)
	{
		UE_LOG_STARTUP(Warning, TEXT("Cannot connect to lobby. AccelByte API Client is invalid."));
		return;
	}

	AccelByte::Api::LobbyPtr LobbyApi = ApiClient->GetLobbyApi().Pin();
	if (!LobbyApi)
	{
		UE_LOG_STARTUP(Warning, TEXT("Cannot connect to lobby. Lobby API is invalid."));
		return;
	}

	LobbyApi->Disconnect(true);
}

void UStartupSubsystem::KillSelf(const TArray<FString>& Args)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	AAccelByteWarsPlayerController* AccelByteWarsPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AccelByteWarsPlayerController)
	{
		return;
	}

	AccelByteWarsPlayerController->ClientInstructInstaKillPlayer({PlayerController->PlayerState});
}

void UStartupSubsystem::KillOthers(const TArray<FString>& Args)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	AAccelByteWarsPlayerController* AccelByteWarsPlayerController = Cast<AAccelByteWarsPlayerController>(PlayerController);
	if (!AccelByteWarsPlayerController)
	{
		return;
	}

	TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;

	// Exclude local user
	const APlayerState* LocalPlayer = GetWorld()->GetFirstPlayerController()->PlayerState;
	PlayerStates.RemoveAll([LocalPlayer](const APlayerState* PlayerState)
	{
		return PlayerState == LocalPlayer;
	});

	AccelByteWarsPlayerController->ClientInstructInstaKillPlayer(PlayerStates);
}

void UStartupSubsystem::OnQueryTeamMembersInfo(
	const APlayerController* PC,
	const TArray<FUniqueNetIdRef>& MemberUserIds,
	const TDelegate<void(const TMap<FUniqueNetIdRepl, FGameplayPlayerData>& TeamMembersInfo)> OnComplete) 
{
	if (!PC) 
	{
		UE_LOG_STARTUP(Warning, TEXT("Failed to query team member info. Player Controller is invalid"));
		OnComplete.ExecuteIfBound({});
		return;
	}

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer) 
	{
		UE_LOG_STARTUP(Warning, TEXT("Failed to query team member info. Local Player is invalid"));
		OnComplete.ExecuteIfBound({});
		return;
	}

	int32 LocalUserNum = LocalPlayer->GetControllerId();
	QueryUserInfo(
		LocalUserNum, 
		MemberUserIds, 
		FOnQueryUsersInfoCompleteDelegate::CreateWeakLambda(this, [this, OnComplete](
		const FOnlineError& Error, const TArray<TSharedPtr<FUserOnlineAccountAccelByte>>& UsersInfo)
		{
			TMap<FUniqueNetIdRepl, FGameplayPlayerData> PlayerDataList;
			for (const TSharedPtr<FUserOnlineAccountAccelByte>& UserInfo : UsersInfo)
			{
				FGameplayPlayerData PlayerData;
				PlayerData.UniqueNetId = UserInfo->GetUserId();
				UserInfo->GetUserAttribute(ACCELBYTE_ACCOUNT_GAME_AVATAR_URL, PlayerData.AvatarURL);

				PlayerDataList.Add(PlayerData.UniqueNetId, PlayerData);
			}
			OnComplete.ExecuteIfBound(PlayerDataList);
		})
	);
}