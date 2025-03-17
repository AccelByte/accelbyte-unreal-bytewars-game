// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "SDKConfigWidget.h"

#include "AccelByteUe4SdkModule.h"
#include "Core/UI/Components/AccelByteWarsWidgetSwitcher.h"

#include "TutorialModuleUtilities/TutorialModuleOnlineUtility.h"

#include "Kismet/GameplayStatics.h"
#include "Components/EditableText.h"
#include "CommonButtonBase.h"

#include "OnlineSubsystemUtils.h"
#include "OnlineIdentityInterfaceAccelByte.h"

void USDKConfigWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	Btn_SaveConfig->OnClicked().AddUObject(this, &ThisClass::SaveSdkConfig);

	Ws_SdkConfig->OnRetryClicked.AddUObject(this, &ThisClass::SaveSdkConfig);
	Ws_SdkConfig->OnCancelClicked.AddUObject(this, &ThisClass::DeactivateWidget);
		
	LoadCurrentSdkConfig();
}

void USDKConfigWidget::NativeOnDeactivated()
{
	Btn_SaveConfig->OnClicked().Clear();

	Ws_SdkConfig->OnRetryClicked.Clear();
	Ws_SdkConfig->OnCancelClicked.Clear();

	Super::NativeOnDeactivated();
}

void USDKConfigWidget::LoadCurrentSdkConfig()
{
	// Load current game client sdk config.
	AccelByte::Settings ClientCreds = IAccelByteUe4SdkModuleInterface::Get().GetClientSettings();
	Edt_GameClientId->SetText(FText::FromString(ClientCreds.ClientId));
	Edt_GameClientSecret->SetText(FText::FromString(ClientCreds.ClientSecret));
	Edt_GameClientNamespace->SetText(FText::FromString(ClientCreds.Namespace));
	Edt_GameClientPublisher->SetText(FText::FromString(ClientCreds.PublisherNamespace));
	Edt_GameClientBaseUrl->SetText(FText::FromString(ClientCreds.BaseUrl));
	Edt_GameClientRedirectUri->SetText(FText::FromString(ClientCreds.RedirectURI));

	// Load current game server sdk config.
	AccelByte::ServerSettings ServerCreds = IAccelByteUe4SdkModuleInterface::Get().GetServerSettings();
	Edt_ServerClientId->SetText(FText::FromString(ServerCreds.ClientId));
	Edt_ServerClientSecret->SetText(FText::FromString(ServerCreds.ClientSecret));
	Edt_ServerNamespace->SetText(FText::FromString(ServerCreds.Namespace));
	Edt_ServerPublisher->SetText(FText::FromString(ServerCreds.PublisherNamespace));
	Edt_ServerBaseUrl->SetText(FText::FromString(ServerCreds.BaseUrl));
	Edt_ServerRedirectUri->SetText(FText::FromString(ServerCreds.RedirectURI));

	// Display default state.
	bIsBackHandler = true;
	Ws_SdkConfig->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Not_Empty);
}

void USDKConfigWidget::SaveSdkConfig()
{
	// Show loading state, cannot back or close the widget at this time.
	bIsBackHandler = false;
	Ws_SdkConfig->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Loading);
	
	// Grab the reference of AccelByte Identity Interface and make sure it's valid.
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to reconfigure SDK config. Invalid AccelByte subsystem."));

		bIsBackHandler = true;
		Ws_SdkConfig->ErrorMessage = FText::FromString(FString("Failed to reconfigure SDK config. Invalid AccelByte subsystem."));
		Ws_SdkConfig->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);

		return;
	}
	FOnlineIdentityAccelBytePtr IdentityInterface = StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
	if (!ensure(IdentityInterface.IsValid()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to reconfigure SDK config. Invalid Identity Interface."));

		bIsBackHandler = true;
		Ws_SdkConfig->ErrorMessage = FText::FromString(FString("Failed to reconfigure SDK config. Invalid Identity Interface."));
		Ws_SdkConfig->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);

		return;
	}

	// Logout first before saving the new sdk config.
	if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::Type::LoggedIn)
	{
		IdentityInterface->AddOnLogoutCompleteDelegate_Handle(0, FOnLogoutCompleteDelegate::CreateWeakLambda(this,
			[this, IdentityInterface](int32 LocalUserNum, bool bWasSuccessful)
			{
				if (!bWasSuccessful)
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to reconfigure SDK config. Failed to logout."));

					bIsBackHandler = true;
					Ws_SdkConfig->SetWidgetState(EAccelByteWarsWidgetSwitcherState::Error);

					return;
				}

				UE_LOG(LogTemp, Log, TEXT("Success to reconfigure SDK config. Game about to be reloaded."));

				IdentityInterface->ClearOnLogoutCompleteDelegates(0, this);

				SaveSdkConfig();
			}
		));
		IdentityInterface->Logout(0);
		return;
	}

	// Save game client sdk configs.
	TMap<FString, FString> ClientSDKConfigs;
	ClientSDKConfigs.Add(FString("ClientId"), Edt_GameClientId->GetText().ToString());
	ClientSDKConfigs.Add(FString("ClientSecret"), Edt_GameClientSecret->GetText().ToString());
	ClientSDKConfigs.Add(FString("Namespace"), Edt_GameClientNamespace->GetText().ToString());
	ClientSDKConfigs.Add(FString("PublisherNamespace"), Edt_GameClientPublisher->GetText().ToString());
	ClientSDKConfigs.Add(FString("BaseUrl"), Edt_GameClientBaseUrl->GetText().ToString());
	ClientSDKConfigs.Add(FString("RedirectURI"), Edt_GameClientRedirectUri->GetText().ToString());

	// Save game server sdk configs.
	TMap<FString, FString> ServerSDKConfigs;
	ServerSDKConfigs.Add(FString("ClientId"), Edt_ServerClientId->GetText().ToString());
	ServerSDKConfigs.Add(FString("ClientSecret"), Edt_ServerClientSecret->GetText().ToString());
	ServerSDKConfigs.Add(FString("Namespace"), Edt_ServerNamespace->GetText().ToString());
	ServerSDKConfigs.Add(FString("PublisherNamespace"), Edt_ServerPublisher->GetText().ToString());
	ServerSDKConfigs.Add(FString("BaseUrl"), Edt_ServerBaseUrl->GetText().ToString());
	ServerSDKConfigs.Add(FString("RedirectURI"), Edt_ServerRedirectUri->GetText().ToString());

	// Override sdk configs.
	UTutorialModuleOnlineUtility::OverrideSDKConfig(ClientSDKConfigs, ServerSDKConfigs);

	// Reload the game.
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("MainMenu"));
}
