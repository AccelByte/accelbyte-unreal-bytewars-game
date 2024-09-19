// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/AccelByteWarsActivatableWidget.h"
#include "OptionsWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnOptionsMenuActivated, const APlayerController* /*Player Controller*/, TDelegate<void()> /*Callback*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnOptionsMenuDeactivated, const APlayerController* /*Player Controller*/, TDelegate<void()> /*Callback*/);

class UAccelByteWarsGameInstance;
class UOptionListEntry_Scalar;
class UOptionListEntry_Toggler;

UCLASS()
class ACCELBYTEWARS_API UOptionsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
// @@@SNIPSTART OptionsWidget.h-public
// @@@MULTISNIP OptionMenuDelegate {"selectedLines": ["1-3"]}
public:
	inline static FOnOptionsMenuActivated OnOptionsWidgetActivated;
	inline static FOnOptionsMenuDeactivated OnOptionsWidgetDeactivated;
// @@@SNIPEND

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
// @@@SNIPSTART OptionsWidget.h-private
// @@@MULTISNIP SoundOptionSliderUI {"selectedLines": ["1", "5-9"]}
private:
	void InitOptions();
	void FinalizeOptions();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Scalar* W_OptionMusicVolumeScalar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Scalar* W_OptionSFXVolumeScalar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Toggler* W_OptionFTUEAlwaysOnToggler;

	UAccelByteWarsGameInstance* GameInstance;
// @@@SNIPEND
};