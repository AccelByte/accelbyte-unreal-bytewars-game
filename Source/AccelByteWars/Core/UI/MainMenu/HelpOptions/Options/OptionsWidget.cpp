// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "Core/UI/MainMenu/HelpOptions/Options/OptionsWidget.h"
#include "Core/UI/MainMenu/HelpOptions/Options/Components/OptionListEntryBase.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/System/AccelByteWarsGameInstance.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UOptionsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
	ensure(GameInstance);

	PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
	ensure(PromptSubsystem);
}

void UOptionsWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// If online, load the online game options.
	if (OnGetOnlineGameOptionsDelegate.IsBound())
	{
		LoadOnlineGameOptions();
	}
	// If offline, load the local game options.
	else 
	{
		InitOptions(nullptr);
	}
}

void UOptionsWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	/* If online, save the online game options.
	 * It is possible that the game was stopped during the options widget was displayed.
	 * When it happens, the owning player will becomes null and the widget will call NativeOnDeactivated().
	 * Thus, the game need to check whether the owning player is valid before saving the online game options. */
	if (GetOwningPlayer() && OnSetOnlineGameOptionsDelegate.IsBound())
	{
		SaveOnlineGameOptions();
	}
	// If offline, save the local game options.
	else 
	{
		FinalizeOptions();
	}
}

void UOptionsWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	MoveCameraToTargetLocation(InDeltaTime, FVector(60.0f, 600.0f, 160.0f));
}

void UOptionsWidget::InitOptions(TUniquePtr<FJsonObject> OptionsData)
{
	// If the passed options data are empty, load local game options.
	if (!OptionsData.IsValid())
	{
		if (!GameInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("Cannot initialize game options. The game instance is not found."));
			return;
		}

		GameInstance->LoadGameSettings();

		OptionsData = MakeUnique<FJsonObject>();
		OptionsData->SetNumberField(SOUND_OPTIONS_MUSIC_KEY, GameInstance->GetMusicVolume());
		OptionsData->SetNumberField(SOUND_OPTIONS_SFX_KEY, GameInstance->GetSFXVolume());
	}

	// Init music volume setting.
	W_OptionMusicVolumeScalar->InitOption(LOCTEXT("Music Setting", "Music"), OptionsData->GetNumberField(SOUND_OPTIONS_MUSIC_KEY));
	W_OptionMusicVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetMusicVolume);

	// Init SFX volume setting.
	W_OptionSFXVolumeScalar->InitOption(LOCTEXT("SFX Setting", "SFX"), OptionsData->GetNumberField(SOUND_OPTIONS_SFX_KEY));
	W_OptionSFXVolumeScalar->OnScalarValueChangedDelegate.AddUObject(GameInstance, &UAccelByteWarsGameInstance::SetSFXVolume);
}

void UOptionsWidget::FinalizeOptions()
{
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot finalize and save game options. The game instance is not found."));
		return;
	}

	W_OptionMusicVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);
	W_OptionSFXVolumeScalar->OnScalarValueChangedDelegate.RemoveAll(GameInstance);

	GameInstance->SaveGameSettings();
}

void UOptionsWidget::LoadOnlineGameOptions()
{
	PromptSubsystem->ShowLoading();

	OnGetOnlineGameOptionsDelegate.Broadcast(
		GetOwningPlayer(),
		FString::Printf(TEXT("%s-%s"), *GAME_OPTIONS_KEY, *SOUND_OPTIONS_KEY),
		FOnGetOnlineGameOptionsComplete::CreateWeakLambda(this, [this](bool bWasSuccessful, FJsonObject& Result)
		{
			// Load online game options if any. If not, then load the local game options.
			InitOptions(bWasSuccessful ? MakeUnique<FJsonObject>(Result) : nullptr);
			PromptSubsystem->HideLoading();
		}
	));
}

void UOptionsWidget::SaveOnlineGameOptions()
{
	PromptSubsystem->ShowLoading(LOCTEXT("Saving", "Saving"));

	// Construct game options to save on Cloud Save.
	FJsonObject NewGameOptions;
	NewGameOptions.SetNumberField(SOUND_OPTIONS_MUSIC_KEY, GameInstance->GetMusicVolume());
	NewGameOptions.SetNumberField(SOUND_OPTIONS_SFX_KEY, GameInstance->GetSFXVolume());

	OnSetOnlineGameOptionsDelegate.Broadcast(
		GetOwningPlayer(),
		FString::Printf(TEXT("%s-%s"), *GAME_OPTIONS_KEY, *SOUND_OPTIONS_KEY),
		NewGameOptions,
		FOnSetOnlineGameOptionsComplete::CreateWeakLambda(this, [this](bool bWasSuccessful)
		{
			// Save the new game options locally.
			FinalizeOptions();
			PromptSubsystem->HideLoading();

			/* If the game options were not successfully saved on Cloud Save,
			 * show a message that the game options are also saved locally. */
			if (!bWasSuccessful) 
			{
				PromptSubsystem->ShowMessagePopUp(
					LOCTEXT("Error", "Error"),
					LOCTEXT("Error Save Online Game Options", "Failed to save game options to Cloud Save. Game options will be saved locally instead.")
				);
			}
		}
	));
}

#undef LOCTEXT_NAMESPACE