// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/UI/MainMenu/HelpOptions/Settings/SettingsWidget.h"
#include "ByteWarsCore/UI/MainMenu/HelpOptions/Settings/Components/SettingsListEntryBase.h"
#include "ByteWarsCore/System/AccelByteWarsGameInstance.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void USettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
}

void USettingsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	InitSettings();
}

void USettingsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	FinalizeSettings();
}

void USettingsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void USettingsWidget::InitSettings()
{
	if (GameInstance == nullptr) 
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot initialize game settings. The game instance is not found."));
		return;
	}

	GameInstance->LoadGameSettings();

	// Init music volume setting.
	float MusicVolume = GameInstance->GetMusicVolume();
	W_SettingMusicVolumeScalar->InitSetting(LOCTEXT("Music Setting", "Music"), MusicVolume);
	W_SettingMusicVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetMusicVolume);

	// Init SFX volume setting.
	float SFXVolume = GameInstance->GetSFXVolume();
	W_SettingSFXVolumeScalar->InitSetting(LOCTEXT("SFX Setting", "SFX"), SFXVolume);
	W_SettingSFXVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetSFXVolume);
}

void USettingsWidget::FinalizeSettings()
{
	if (GameInstance == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot finalize and save game settings. The game instance is not found."));
		return;
	}

	W_SettingMusicVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);
	W_SettingSFXVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);

	GameInstance->SaveGameSettings();
}

#undef LOCTEXT_NAMESPACE