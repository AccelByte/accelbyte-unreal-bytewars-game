// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "Core/UI/Components/Info/InfoWidget.h"

#include "OnlineSubsystemUtils.h"
#include "PluginDescriptor.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/KismetSystemLibrary.h"

void UInfoWidget::RefreshUI()
{
	DisplayInfo();
}

void UInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	DisplayInfo();
}

void UInfoWidget::DisplayInfo()
{
	// user info
	const APlayerController* OwningPlayerController = GetOwningPlayer();
	if (!OwningPlayerController)
	{
		goto UserInfoEmpty;
	}
	const APlayerState* OwningPlayerState = OwningPlayerController->PlayerState;
	if (!OwningPlayerState)
	{
		goto UserInfoEmpty;
	}

	if (!OwningPlayerState->GetUniqueId().GetType().IsEqual("NULL"))
	{
		/**
		 * Player logged in, displays name and ID.
		 * For some reason, AB user ID does not Immidiately stored in the PlayerState.UniqueNetId.
		 * Use Identity Interface as a workaround
		 */
		 // Get owning player's local user index
		const ULocalPlayer* OwningLocalPlayer = OwningPlayerController->GetLocalPlayer();
		if (!OwningLocalPlayer)
		{
			goto UserInfoEmpty;
		}
		const FUniqueNetIdPtr OwningNetId = Online::GetSubsystem(
			GetWorld())->GetIdentityInterface()->GetUniquePlayerId(OwningLocalPlayer->GetLocalPlayerIndex());
		if (!OwningNetId.IsValid())
		{
			goto UserInfoEmpty;
		}

		const FString UserNickname = GetOwningPlayer()->PlayerState->GetPlayerName();
		const FString UserId = OwningNetId->ToDebugString();

		Text_Username->SetText(FText::FromString(UserNickname));
		Text_UserId->SetVisibility(ESlateVisibility::Visible);
		Text_UserId->SetText(FText::FromString(UserId));
	}
	else
	{
		UserInfoEmpty:;

		// player not logged in, displays name as 'not logged in'
		Text_Username->SetText(FText::FromString("Not logged in"));
		Text_UserId->SetVisibility(ESlateVisibility::Collapsed);
	}

	// build info
	const FString BuildInfo = FString::Printf(TEXT("%s (%s)"),
		*UKismetSystemLibrary::GetBuildVersion(),
		*UKismetSystemLibrary::GetBuildConfiguration());
	Text_BuildInfo->SetText(FText::FromString(BuildInfo));

	// Project info
	FString ProjectVersion = "";
	GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), ProjectVersion, GGameIni);

	// Plugins
	FString PluginsInfo = "";
	IPluginManager& PluginManager = IPluginManager::Get();
	const TArray<TSharedRef<IPlugin>> Plugins = PluginManager.GetDiscoveredPlugins();

	bool bIsBaseGame = true;
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
	Text_ProjectInfo->SetText(FText::FromString(FString::Printf(
		TEXT("Version: %s (%s)%s"),
		*ProjectVersion,
		*FString(bIsBaseGame ? "BaseGame" : "TutorialModules"),
		*PluginsInfo)));
}