// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "NativeFriendsSubsystem_Starter.h"
#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/UI/Components/Prompt/PromptSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "TutorialModuleUtilities/StartupSubsystem.h"

void UNativeFriendsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    // Get Online Subsystem and make sure it's valid.
    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
        return;
    }

    // Grab the reference of AccelByte User Interface and make sure it's valid.
    UserInterface = StaticCastSharedPtr<FOnlineUserAccelByte>(Subsystem->GetUserInterface());
    if (!ensure(UserInterface.IsValid()))
    {
        UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("User Interface is not valid."));
        return;
    }

    // Grab the reference of AccelByte Friends Interface and make sure it's valid.
    FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
    if (!ensure(FriendsInterface.IsValid()))
    {
        UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("Friends Interface is not valid."));
        return;
    }

    // Cast Online Subsystem to AccelByte OSS.
    ABSubsystem = static_cast<FOnlineSubsystemAccelByte*>(Subsystem);
    if (!ensure(ABSubsystem))
    {
        UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The AccelByte online subsystem is invalid."));
        return;
    }

    // Grab the reference of AccelByte User Cache Interface and make sure it's valid.
    UserCache = StaticCastSharedPtr<FOnlineUserCacheAccelByte>(ABSubsystem->GetUserCache());
    if (!ensure(UserCache))
    {
        UE_LOG_NATIVE_FRIENDS_ESSENTIALS(Warning, TEXT("The User Cache Interface is invalid."));
        return;
    }

    // Grab prompt subsystem.
    UAccelByteWarsGameInstance* GameInstance = Cast<UAccelByteWarsGameInstance>(GetGameInstance());
    ensure(GameInstance);

    PromptSubsystem = GameInstance->GetSubsystem<UPromptSubsystem>();
    ensure(PromptSubsystem);
}

void UNativeFriendsSubsystem_Starter::Deinitialize()
{
    Super::Deinitialize();
}

FUniqueNetIdPtr UNativeFriendsSubsystem_Starter::GetUniqueNetIdFromPlayerController(const APlayerController* PC) const
{
    if (!PC)
    {
        return nullptr;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return nullptr;
    }

    return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

int32 UNativeFriendsSubsystem_Starter::GetLocalUserNumFromPlayerController(const APlayerController* PC) const
{
    int32 LocalUserNum = 0;

    if (!PC)
    {
        return LocalUserNum;
    }

    const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (LocalPlayer)
    {
        LocalUserNum = LocalPlayer->GetControllerId();
    }

    return LocalUserNum;
}

#pragma region Module Get Native Friend List Function Definitions

// TODO: Add your Module Get Native Friend List function definitions here.

#pragma endregion


#pragma region Module Sync Native Friend List Function Definitons

// TODO: Add your Module Sync Native Friend List function definitions here.

#pragma endregion
