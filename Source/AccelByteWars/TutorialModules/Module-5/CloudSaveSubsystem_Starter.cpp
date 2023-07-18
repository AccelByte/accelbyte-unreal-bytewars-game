// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-5/CloudSaveSubsystem_Starter.h"
#include "TutorialModules/Module-2/AuthEssentialsModels.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "Core/UI/MainMenu/HelpOptions/Options/OptionsWidget.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UCloudSaveSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const FOnlineSubsystemAccelByte* Subsystem = static_cast<const FOnlineSubsystemAccelByte*>(Online::GetSubsystem(GetWorld()));
    if (!ensure(Subsystem))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte Identity Interface and make sure it's valid.
    CloudSaveInterface = StaticCastSharedPtr<FOnlineCloudSaveAccelByte>(Subsystem->GetCloudSaveInterface());
    if (!ensure(CloudSaveInterface.IsValid()))
    {
        UE_LOG_CLOUDSAVE_ESSENTIALS(Warning, TEXT("Cloud Save interface is not valid."));
        return;
    }

    BindDelegates();
}

void UCloudSaveSubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();

    UnbindDelegates();
}


#pragma region Module.5 General Function Definitions
void UCloudSaveSubsystem_Starter::BindDelegates()
{
    // TODO: Bind your delegates here.
}

void UCloudSaveSubsystem_Starter::UnbindDelegates()
{
    // TODO: Unbind your delegates here.
}

void UCloudSaveSubsystem_Starter::OnLoadGameSoundOptions(const APlayerController* PC, TDelegate<void()> OnComplete)
{
    // TODO: Implement to load game sound options from Cloud Save here.
}

void UCloudSaveSubsystem_Starter::OnSaveGameSoundOptions(const APlayerController* PC, TDelegate<void()> OnComplete)
{
    // TODO: Implement to save game sound options to Cloud Save here.
}
#pragma endregion


#pragma region Module.5 Function Definitions

// TODO: Add your Module.5 function definitions here.

#pragma endregion

#undef LOCTEXT_NAMESPACE