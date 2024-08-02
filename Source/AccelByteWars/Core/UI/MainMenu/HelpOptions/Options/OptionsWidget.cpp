// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/HelpOptions/Options/OptionsWidget.h"
#include "Core/UI/MainMenu/HelpOptions/Options/Components/OptionListEntryBase.h"
#include "Core/System/AccelByteWarsGameInstance.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);
}

void UOptionsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	InitOptions();

	if (OnOptionsWidgetActivated.IsBound())
	{
		OnOptionsWidgetActivated.Broadcast(GetOwningPlayer(), TDelegate<void()>::CreateUObject(this, &ThisClass::InitOptions));
	}
}

void UOptionsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	FinalizeOptions();

	if (OnOptionsWidgetDeactivated.IsBound()) 
	{
		OnOptionsWidgetDeactivated.Broadcast(GetOwningPlayer(), TDelegate<void()>::CreateUObject(this, &ThisClass::FinalizeOptions));
	}
}

void UOptionsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void UOptionsWidget::InitOptions()
{
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot initialize game options. The game instance is null."));
		return;
	}

	// Init music volume setting.
	W_OptionMusicVolumeScalar->InitOption(LOCTEXT("Music Setting", "Music"), GameInstance->GetMusicVolume());
	W_OptionMusicVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetMusicVolume);

	// Init SFX volume setting.
	W_OptionSFXVolumeScalar->InitOption(LOCTEXT("SFX Setting", "SFX"), GameInstance->GetSFXVolume());
	W_OptionSFXVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetSFXVolume);

	// Init FTUE setting.
	W_OptionFTUEAlwaysOnToggler->InitOption(LOCTEXT("Show FTUE Setting", "Always Show FTUE"), GameInstance->GetFTUEAlwaysOnSetting());
	W_OptionFTUEAlwaysOnToggler->OnToggleValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetFTUEAlwaysOnSetting);
}

void UOptionsWidget::FinalizeOptions()
{
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot finalize and save game options. The game instance is null."));
		return;
	}

	W_OptionMusicVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);
	W_OptionSFXVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);
	W_OptionFTUEAlwaysOnToggler->OnToggleValueChangedDelegate.RemoveAll(GameInstance);

	const ULocalPlayer* LocalPlayer = GetOwningPlayer()->GetLocalPlayer();
	ensure(LocalPlayer != nullptr);
	const int32 LocalUserNum = LocalPlayer->GetControllerId();

	GameInstance->SaveGameSettings(LocalUserNum);
}

#undef LOCTEXT_NAMESPACE