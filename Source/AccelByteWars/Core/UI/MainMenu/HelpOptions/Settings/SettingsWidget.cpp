// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/HelpOptions/Settings/SettingsWidget.h"
#include "Core/UI/MainMenu/HelpOptions/Settings/Components/SettingsListEntryBase.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/System/AccelByteWarsGameInstance.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void USettingsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// If online, load the online game settings.
	if (OnGetOnlineGameSettingsDelegate.IsBound())
	{
		LoadOnlineGameSettings();
	}
	// If offline, load the local game settings.
	else 
	{
		InitSettings(nullptr);
	}
}

void USettingsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// If online, save the online game settings.
	if (OnSetOnlineGameSettingsDelegate.IsBound()) 
	{
		SaveOnlineGameSettings();
	}
	// If offline, save the local game settings.
	else 
	{
		FinalizeSettings();
	}
}

void USettingsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void USettingsWidget::InitSettings(TUniquePtr<FJsonObject> SettingsData)
{
	// If the passed settings data are empty, load local game settings.
	if (!SettingsData.IsValid())
	{
		if (!GameInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot initialize game settings. The game instance is not found."));
			return;
		}

		GameInstance->LoadGameSettings();

		SettingsData = MakeUnique<FJsonObject>();
		SettingsData->SetNumberField(SOUND_OPTIONS_MUSIC_KEY, GameInstance->GetMusicVolume());
		SettingsData->SetNumberField(SOUND_OPTIONS_SFX_KEY, GameInstance->GetSFXVolume());
	}

	// Init music volume setting.
	W_SettingMusicVolumeScalar->InitSetting(LOCTEXT("Music Setting", "Music"), SettingsData->GetNumberField(SOUND_OPTIONS_MUSIC_KEY));
	W_SettingMusicVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetMusicVolume);

	// Init SFX volume setting.
	W_SettingSFXVolumeScalar->InitSetting(LOCTEXT("SFX Setting", "SFX"), SettingsData->GetNumberField(SOUND_OPTIONS_SFX_KEY));
	W_SettingSFXVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetSFXVolume);
}

void USettingsWidget::FinalizeSettings()
{
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot finalize and save game settings. The game instance is not found."));
		return;
	}

	W_SettingMusicVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);
	W_SettingSFXVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);

	GameInstance->SaveGameSettings();
}

void USettingsWidget::LoadOnlineGameSettings()
{
	PromptSubsystem->ShowLoading();

	OnGetOnlineGameSettingsDelegate.Broadcast(
		GetOwningPlayer(),
		FString::Printf(TEXT("%s-%s"), *GAME_OPTIONS_KEY, *SOUND_OPTIONS_KEY),
		FOnGetOnlineGameSettingsComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, FJsonObject& Result)
		{
			// Load online game settings if any. If not, then load the local game settings.
			InitSettings(bWasSuccessful ? MakeUnique<FJsonObject>(Result) : nullptr);
			PromptSubsystem->HideLoading();
		}
	));
}

void USettingsWidget::SaveOnlineGameSettings()
{
	PromptSubsystem->ShowLoading(LOCTEXT("Saving", "Saving"));

	// Construct game settings to save on Cloud Save.
	FJsonObject NewGameSettings;
	NewGameSettings.SetNumberField(SOUND_OPTIONS_MUSIC_KEY, GameInstance->GetMusicVolume());
	NewGameSettings.SetNumberField(SOUND_OPTIONS_SFX_KEY, GameInstance->GetSFXVolume());

	OnSetOnlineGameSettingsDelegate.Broadcast(
		GetOwningPlayer(),
		FString::Printf(TEXT("%s-%s"), *GAME_OPTIONS_KEY, *SOUND_OPTIONS_KEY),
		NewGameSettings,
		FOnSetOnlineGameSettingsComplete::CreateWeakLambda(this, [this](bool bWasSuccessful)
		{
			// Save the new game settings locally.
			FinalizeSettings();
			PromptSubsystem->HideLoading();

			/* If the game settings were not successfully saved on Cloud Save,
			 * show a message that the game settings are also saved locally. */
			if (!bWasSuccessful) 
			{
				PromptSubsystem->ShowMessagePopUp(
					LOCTEXT("Error", "Error"),
					LOCTEXT("Error Save Online Game Settings", "Failed to save game settings to Cloud Save. Game settings will be saved locally instead.")
				);
			}
		}
	));
}

#undef LOCTEXT_NAMESPACE