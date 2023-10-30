// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/Components/Info/InfoWidget.h"

#include "OnlineSubsystemUtils.h"
#include "PluginDescriptor.h"
#include "Components/TextBlock.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleDataAsset.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleUtility.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetSystemLibrary.h"

void UInfoWidget::RefreshUI()
{
#pragma region "Is in demo mode or not"
	bool bDemoMode = false;
	bool bDemoModeSet = false;

	// launch param
	if (StaticCast<FString>(FCommandLine::Get()).Contains(TEXT("-DemoMode="), ESearchCase::IgnoreCase))
	{
		FString DemoMode;
		FParse::Value(FCommandLine::Get(), TEXT("-DemoMode="), DemoMode);
		if (DemoMode.Equals("true", ESearchCase::IgnoreCase))
		{
			bDemoMode = true;
			bDemoModeSet = true;
		}
		else if (DemoMode.Equals("false", ESearchCase::IgnoreCase))
		{
			bDemoMode = false;
			bDemoModeSet = true;
		}
	}

	// config DefaultEngine.ini
	if (!bDemoModeSet)
	{
		GConfig->GetBool(TEXT("ByteWars"), TEXT("bDemoMode"), bDemoMode, GEngineIni);
	}
#pragma endregion

	// in demo mode, show only on login menu
	if (bDemoMode)
	{
		if (UTutorialModuleDataAsset* DA = UTutorialModuleUtility::GetTutorialModuleDataAsset(
			FPrimaryAssetId{ "TutorialModule:AUTHESSENTIALS" }, this, true))
		{
			const bool bIsInLoginMenu = DA->GetTutorialModuleUIClass()->IsChildOf(
				GetFirstOccurenceOuter<UAccelByteWarsActivatableWidget>()->GetClass());
			SetVisibility(bIsInLoginMenu ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
	}

	// user info
	if (bDemoMode)
	{
		Text_Username->SetVisibility(ESlateVisibility::Collapsed);
		Text_UserId->SetVisibility(ESlateVisibility::Collapsed);
		Text_BuildInfo->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		FString UserNickname;
		FString UserId;
		if (GetUserInfo(UserNickname, UserId))
		{
			Text_Username->SetText(FText::FromString(UserNickname));
			Text_UserId->SetVisibility(ESlateVisibility::Visible);
			Text_UserId->SetText(FText::FromString(UserId));
		}
		else
		{
			// player not logged in, displays name as 'not logged in'
			Text_Username->SetText(FText::FromString("Not logged in"));
			Text_UserId->SetVisibility(ESlateVisibility::Collapsed);
		}

		// build info
		const FString BuildInfo = FString::Printf(TEXT("%s (%s)"),
			*UKismetSystemLibrary::GetBuildVersion(),
			*UKismetSystemLibrary::GetBuildConfiguration());
		Text_BuildInfo->SetText(FText::FromString(BuildInfo));
	}

	FString ProjectVersion = "";
	FString PluginsInfo = "";
	bool bIsBaseGame = true;

	// Project info
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), ProjectVersion, GGameIni);

	// Plugins
	IPluginManager& PluginManager = IPluginManager::Get();
	const TArray<TSharedRef<IPlugin>> Plugins = PluginManager.GetDiscoveredPlugins();
	for (const TSharedRef<IPlugin>& Plugin : Plugins)
	{
		const FPluginDescriptor& PluginDescriptor = Plugin->GetDescriptor();
		if (PluginDescriptor.CreatedBy.Contains("AccelByte"))
		{
			bIsBaseGame = false;
			PluginsInfo.Appendf(
				TEXT("%sPlugin: %s Version: %s"),
				LINE_TERMINATOR,
				*Plugin->GetName(),
				*PluginDescriptor.VersionName);
		}
	}

	// set text
	const FString ProjectInfoFinal = bDemoMode ?
		FString::Printf(
			TEXT("Version: %s"),
			*ProjectVersion) :
		FString::Printf(
			TEXT("Version: %s (%s)%s"),
			*ProjectVersion,
			*FString(bIsBaseGame ? "BaseGame" : "TutorialModules"),
			*PluginsInfo);
	Text_ProjectInfo->SetText(FText::FromString(ProjectInfoFinal));
}

void UInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshUI();
}

bool UInfoWidget::GetUserInfo(FString& OutUserNickname, FString& OutUserId) const
{
	// user info
	const APlayerController* OwningPlayerController = GetOwningPlayer();
	if (!OwningPlayerController)
	{
		return false;
	}
	const APlayerState* OwningPlayerState = OwningPlayerController->PlayerState;
	if (!OwningPlayerState)
	{
		return false;
	}

	if (OwningPlayerState->GetUniqueId().GetType().IsEqual("NULL"))
	{
		return false;
	}
	
	/**
	 * Player logged in, displays name and ID.
	 * For some reason, AB user ID does not Immidiately stored in the PlayerState.UniqueNetId.
	 * Use Identity Interface as a workaround
	 */
	// Get owning player's local user index
	const ULocalPlayer* OwningLocalPlayer = OwningPlayerController->GetLocalPlayer();
	if (!OwningLocalPlayer)
	{
		return false;
	}
	const FUniqueNetIdPtr OwningNetId = Online::GetSubsystem(
		GetWorld())->GetIdentityInterface()->GetUniquePlayerId(OwningLocalPlayer->GetLocalPlayerIndex());
	if (!OwningNetId.IsValid())
	{
		return false;
	}

	OutUserNickname = GetOwningPlayer()->PlayerState->GetPlayerName();
	OutUserId = OwningNetId->ToDebugString();

	return true;
}
