// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "SettingsWidget.generated.h"

#define GAME_OPTIONS_KEY FString(TEXT("GameOptions"))
#define SOUND_OPTIONS_KEY FString(TEXT("Sound"))
#define SOUND_OPTIONS_MUSIC_KEY FString(TEXT("musicvolume"))
#define SOUND_OPTIONS_SFX_KEY FString(TEXT("sfxvolume"))

DECLARE_DELEGATE_TwoParams(FOnGetOnlineGameSettingsComplete, bool /*bWasSuccessful*/, FJsonObject& /*Result*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetOnlineGameSettings, const APlayerController* /*Player Controller*/, const FString& /*RecordKey*/, const FOnGetOnlineGameSettingsComplete& /*OnGetGameSettingsComplete*/);

DECLARE_DELEGATE_OneParam(FOnSetOnlineGameSettingsComplete, bool /*bWasSuccessful*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnSetOnlineGameSettings, const APlayerController* /*Player Controller*/, const FString& /*RecordKey*/, const FJsonObject& /*RecordData*/, const FOnSetOnlineGameSettingsComplete& /*OnSetGameSettingsComplete*/);

class UAccelByteWarsGameInstance;
class UPromptSubsystem;
class USettingsListEntry_Scalar;

UCLASS()
class ACCELBYTEWARS_API USettingsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	inline static FOnGetOnlineGameSettings OnGetOnlineGameSettingsDelegate;
	inline static FOnSetOnlineGameSettings OnSetOnlineGameSettingsDelegate;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void InitSettings(TUniquePtr<FJsonObject> SettingsData);
	void FinalizeSettings();

	void LoadOnlineGameSettings();
	void SaveOnlineGameSettings();

	UPromptSubsystem* PromptSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	USettingsListEntry_Scalar* W_SettingMusicVolumeScalar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	USettingsListEntry_Scalar* W_SettingSFXVolumeScalar;

	UAccelByteWarsGameInstance* GameInstance;
};