// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "RecentPlayersSubsystem_Starter.h"
#include "RecentPlayersLog.h"

#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineSubsystemUtils.h"

#include "Social/FriendsEssentials/FriendsSubsystem.h"
#include "Social/ManagingFriends/ManagingFriendsSubsystem.h"

#include "Core/System/AccelByteWarsGameInstance.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

void URecentPlayersSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    if (!ensure(Subsystem))
    {
        UE_LOG_RECENTPLAYERS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and DefaultPlatformService under [OnlineSubsystem] in the Engine.ini set to AccelByte."));
        return;
    }

    FriendsInterface = StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
    if (!ensure(FriendsInterface.IsValid()))
    {
        UE_LOG_RECENTPLAYERS(Warning, TEXT("Friends Interface is not valid."));
        return;
    }

    SessionInterface = Subsystem->GetSessionInterface();
    if (!ensure(SessionInterface.IsValid()))
    {
        UE_LOG_RECENTPLAYERS(Warning, TEXT("Session Interface is not valid."));
        return;
    }

    // TODO: Add your Module Recent Players code here.
}

void URecentPlayersSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

    // TODO: Add your Module Recent Players code here.
}

FUniqueNetIdPtr URecentPlayersSubsystem_Starter::GetUniqueNetIdFromPlayerController(const APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return nullptr;
    }

    const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return nullptr;
    }

    return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

#pragma region Module Recent Players Definitions
// TODO: Add your Module Recent Players code here.
#pragma endregion
