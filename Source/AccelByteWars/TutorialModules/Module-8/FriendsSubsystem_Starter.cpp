// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "TutorialModules/Module-8/FriendsSubsystem_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"

#define LOCTEXT_NAMESPACE "AccelByteWars"

void UFriendsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte User Interface and make sure it's valid.
    UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
    if (!ensure(UserInterface.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("User Interface is not valid."));
        return;
    }

    // Grab the reference of AccelByte Friends Interface and make sure it's valid.
    FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
    if (!ensure(FriendsInterface.IsValid()))
    {
        UE_LOG_FRIENDS_ESSENTIALS(Warning, TEXT("Friends Interface is not valid."));
        return;
    }

    // Grab prompt subsystem.
    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    ensure(GameInstance);

    PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
    ensure(PromptSubsystem);
}

void UFriendsSubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();
}

FUniqueNetIdPtr UFriendsSubsystem_Starter::GetPlayerUniqueNetId(const APlayerController* PC) const
{
    if (!ensure(PC))
    {
        return nullptr;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!ensure(LocalPlayer))
    {
        return nullptr;
    }

    return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

int32 UFriendsSubsystem_Starter::GetPlayerControllerId(const APlayerController* PC) const
{
    int32 LocalUserNum = 0;

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (LocalPlayer)
    {
        LocalUserNum = LocalPlayer->GetControllerId();
    }

    return LocalUserNum;
}


#pragma region Module.8a Function Definitions

// TODO: Add your Module.8a function definitions here.

#pragma endregion


#pragma region Module.8b Function Definitions

// TODO: Add your Module.8b function definitions here.

#pragma endregion


#pragma region Module.8c Function Definitions

// TODO: Add your Module.8c function definitions here.

#pragma endregion


#undef LOCTEXT_NAMESPACE