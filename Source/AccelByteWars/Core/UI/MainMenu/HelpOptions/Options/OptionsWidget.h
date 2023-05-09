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

UCLASS()
class ACCELBYTEWARS_API UOptionsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	inline static FOnOptionsMenuActivated OnOptionsWidgetActivated;
	inline static FOnOptionsMenuDeactivated OnOptionsWidgetDeactivated;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void InitOptions();
	void FinalizeOptions();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Scalar* W_OptionMusicVolumeScalar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UOptionListEntry_Scalar* W_OptionSFXVolumeScalar;

	UAccelByteWarsGameInstance* GameInstance;
};