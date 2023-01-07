// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "SettingsWidget.generated.h"

class UAccelByteWarsGameInstance;
class USettingsListEntry_Scalar;

UCLASS()
class ACCELBYTEWARS_API USettingsWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	USettingsListEntry_Scalar* W_SettingMusicVolumeScalar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	USettingsListEntry_Scalar* W_SettingSFXVolumeScalar;

protected:
	UAccelByteWarsGameInstance* GameInstance;

protected:
	void NativeConstruct() override;
	void NativeOnActivated() override;
	void NativeOnDeactivated() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitSettings();
	void FinalizeSettings();
};
