// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OptionsWidget.generated.h"

#define GAME_OPTIONS_KEY FString(TEXT("GameOptions"))
#define SOUND_OPTIONS_KEY FString(TEXT("Sound"))
#define SOUND_OPTIONS_MUSIC_KEY FString(TEXT("musicvolume"))
#define SOUND_OPTIONS_SFX_KEY FString(TEXT("sfxvolume"))

DECLARE_DELEGATE_TwoParams(FOnGetOnlineGameOptionsComplete, bool /*bWasSuccessful*/, FJsonObject& /*Result*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGetOnlineGameOptions, const APlayerController* /*Player Controller*/, const FString& /*RecordKey*/, const FOnGetOnlineGameOptionsComplete& /*OnGetGameOptionsComplete*/);

DECLARE_DELEGATE_OneParam(FOnSetOnlineGameOptionsComplete, bool /*bWasSuccessful*/);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnSetOnlineGameOptions, const APlayerController* /*Player Controller*/, const FString& /*RecordKey*/, const FJsonObject& /*RecordData*/, const FOnSetOnlineGameOptionsComplete& /*OnSetGameOptionsComplete*/);

class UAccelByteWarsGameInstance;
class UPromptSubsystem;
class UOptionListEntry_Scalar;

UCLASS()
class ACCELBYTEWARS_API UOptionsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	inline static FOnGetOnlineGameOptions OnGetOnlineGameOptionsDelegate;
	inline static FOnSetOnlineGameOptions OnSetOnlineGameOptionsDelegate;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void InitOptions(TUniquePtr<FJsonObject> OptionsData);
	void FinalizeOptions();

	void LoadOnlineGameOptions();
	void SaveOnlineGameOptions();

	UPromptSubsystem* PromptSubsystem;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Scalar* W_OptionMusicVolumeScalar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Scalar* W_OptionSFXVolumeScalar;

	UAccelByteWarsGameInstance* GameInstance;
};