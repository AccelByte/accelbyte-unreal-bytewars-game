// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "AccelByteWarsPlatformEmulation.generated.h"

UCLASS(config = EditorPerProjectUserSettings, MinimalAPI)
class UAccelByteWarsPlatformEmulation : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override;
	virtual void PostInitProperties() override;
#endif

	virtual FName GetCategoryName() const override;

	FName GetActivateEmulatedPlatformName() const;
	FName GetBaseDeviceProfileName() const;

private:
#if WITH_EDITOR
	void ApplyPlatformEmulationSettings();
	void ActivateEmulatedPlatform(FName NewPlatformName);
	void ActivateEmulatedDeviceProfile();
#endif

	void SetDefaultBaseDeviceProfile();

	UFUNCTION()
	TArray<FName> GetPlatformIds() const;

	UFUNCTION()
	TArray<FName> GetDeviceProfiles() const;

	UPROPERTY(EditAnywhere, config, Category = PlatformEmulation, meta = (GetOptions = GetPlatformIds))
	FName EmulatedPlatform;

	UPROPERTY(EditAnywhere, config, Category = PlatformEmulation, meta = (GetOptions = GetDeviceProfiles, EditCondition = bApplyDeviceProfiles))
	FName BaseDeviceProfile;

	UPROPERTY(EditAnywhere, config, Category = PlatformEmulation, meta = (InlineEditConditionToggle))
	bool bApplyDeviceProfiles = false;

	FName LastActivateEmulatedPlatform;
	FString CurrentAppliedDeviceProfile;
};