// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ByteWarsCore/System/AccelByteWarsPlatformEmulation.h"
#include "Engine/PlatformSettings.h"
#include "Misc/App.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "DeviceProfiles/DeviceProfile.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#if WITH_EDITOR
void UAccelByteWarsPlatformEmulation::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    ApplyPlatformEmulationSettings();
}

void UAccelByteWarsPlatformEmulation::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
    Super::PostReloadConfig(PropertyThatWasLoaded);
    ApplyPlatformEmulationSettings();
}

void UAccelByteWarsPlatformEmulation::PostInitProperties()
{
    Super::PostInitProperties();
    ApplyPlatformEmulationSettings();
}
#endif

FName UAccelByteWarsPlatformEmulation::GetCategoryName() const
{
    return FApp::GetProjectName();
}

FName UAccelByteWarsPlatformEmulation::GetActivateEmulatedPlatformName() const
{
    return EmulatedPlatform;
}

FName UAccelByteWarsPlatformEmulation::GetBaseDeviceProfileName() const
{
    return BaseDeviceProfile;
}

#if WITH_EDITOR
void UAccelByteWarsPlatformEmulation::ApplyPlatformEmulationSettings()
{
    if (GIsEditor && EmulatedPlatform != LastActivateEmulatedPlatform)
    {
        ActivateEmulatedPlatform(EmulatedPlatform);
    }

    SetDefaultBaseDeviceProfile();
    ActivateEmulatedDeviceProfile();
}

void UAccelByteWarsPlatformEmulation::ActivateEmulatedPlatform(FName NewPlatformName)
{
    LastActivateEmulatedPlatform = NewPlatformName;
    EmulatedPlatform = NewPlatformName;

    UPlatformSettingsManager::SetEditorSimulatedPlatform(EmulatedPlatform);
}

void UAccelByteWarsPlatformEmulation::ActivateEmulatedDeviceProfile()
{
    // If not applying any device profile, then set the device profile to none.
    if (bApplyDeviceProfiles == false)
    {
        BaseDeviceProfile = NAME_None;
    }

    // Get the default platform device profile, then override it to the desired device profile if any.
    FString ProfileToApply = UDeviceProfileManager::GetPlatformDeviceProfileName();
#if WITH_EDITOR
    if (GIsEditor && BaseDeviceProfile != NAME_None)
    {
        ProfileToApply = BaseDeviceProfile.ToString();
    }
#endif

    // If the profile is not yet created, then create one.
    UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
    if (Manager.HasLoadableProfileName(ProfileToApply, EmulatedPlatform))
    {
        UDeviceProfile* Profile = Manager.FindProfile(ProfileToApply, false);
        if (Profile == nullptr)
        {
            Profile = Manager.CreateProfile(ProfileToApply, TEXT(""), ProfileToApply, *EmulatedPlatform.ToString());
        }
    }

    // Restore to default device profiles first.
    if (GIsEditor)
    {
#if ALLOW_OTHER_PLATFORM_CONFIG
        Manager.RestorePreviewDeviceProfile();
#endif
    }
    else
    {
        Manager.RestoreDefaultDeviceProfile();
    }

    // Apply the new profile.
    UDeviceProfile* NewProfile = Manager.FindProfile(ProfileToApply);
    ensureMsgf(NewProfile != nullptr, TEXT("Cannot find Device Profile %s."), *ProfileToApply);
    if (NewProfile)
    {
        if (GIsEditor)
        {
#if ALLOW_OTHER_PLATFORM_CONFIG
            UE_LOG(LogConsoleResponse, Log, TEXT("[Preview] Device Profile overridden to %s."), *ProfileToApply);
            Manager.SetPreviewDeviceProfile(NewProfile);
#endif
        }
        else
        {
            UE_LOG(LogConsoleResponse, Log, TEXT("Device Profile overridden to %s."), *ProfileToApply);
            Manager.SetOverrideDeviceProfile(NewProfile);
        }
    }
}
#endif

void UAccelByteWarsPlatformEmulation::SetDefaultBaseDeviceProfile()
{
    // Check if the current device profile is compatible with the emulated platform, if not, set it to none.
    UDeviceProfileManager& Manager = UDeviceProfileManager::Get();
    if (UDeviceProfile* ProfilePtr = Manager.FindProfile(BaseDeviceProfile.ToString(), false))
    {
        const bool bIsCompatible = (EmulatedPlatform == NAME_None) || (ProfilePtr->DeviceType == EmulatedPlatform.ToString());
        if (!bIsCompatible)
        {
            BaseDeviceProfile = NAME_None;
        }
    }

    // If emulate a platform but don't have a base device profile. Then, choose the closest one.
    if ((EmulatedPlatform != NAME_None) && (BaseDeviceProfile == NAME_None))
    {
        FName ClosestMatchingProfile;
        for (const TObjectPtr<UDeviceProfile>& DeviceProfile : Manager.Profiles)
        {
            if (DeviceProfile->DeviceType == EmulatedPlatform.ToString())
            {
                const FName TempProfile = DeviceProfile->GetFName();
                if ((ClosestMatchingProfile == NAME_None) || (TempProfile.GetStringLength() < ClosestMatchingProfile.GetStringLength()))
                {
                    ClosestMatchingProfile = TempProfile;
                }
            }
        }
        BaseDeviceProfile = ClosestMatchingProfile;
    }
}

TArray<FName> UAccelByteWarsPlatformEmulation::GetPlatformIds() const
{
    TArray<FName> PlatformIds;

#if WITH_EDITOR
    PlatformIds.Add(NAME_None);
    PlatformIds.Append(UPlatformSettingsManager::GetKnownAndEnablePlatformIniNames());
#endif

    return PlatformIds;
}

TArray<FName> UAccelByteWarsPlatformEmulation::GetDeviceProfiles() const
{
    TArray<FName> DeviceProfiles;

#if WITH_EDITOR
    const UDeviceProfileManager& DeviceProfileManager = UDeviceProfileManager::Get();
    DeviceProfiles.Reserve(DeviceProfileManager.Profiles.Num() + 1);

    if (EmulatedPlatform == NAME_None)
    {
        DeviceProfiles.Add(NAME_None);
    }

    for (const TObjectPtr<UDeviceProfile>& DeviceProfile : DeviceProfileManager.Profiles)
    {
        if (!(EmulatedPlatform != NAME_None && DeviceProfile->DeviceType != EmulatedPlatform.ToString()))
        {
            DeviceProfiles.Add(DeviceProfile->GetFName());
        }
    }
#endif

    return DeviceProfiles;
}