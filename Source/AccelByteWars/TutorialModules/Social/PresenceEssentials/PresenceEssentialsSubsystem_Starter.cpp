// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "PresenceEssentialsSubsystem_Starter.h"

#include "OnlineFriendsInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "OnlineSubsystemUtils.h"

#include "Core/System/AccelByteWarsGameInstance.h"
#include "Core/GameStates/AccelByteWarsGameState.h"
#include "Core/GameStates/AccelByteWarsMainMenuGameState.h"
#include "Core/GameStates/AccelByteWarsInGameGameState.h"
#include "Core/UI/AccelByteWarsBaseUI.h"

#include "Play/OnlineSessionUtils/AccelByteWarsOnlineSessionBase.h"

void UPresenceEssentialsSubsystem_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// TODO: Add your code here.
}

void UPresenceEssentialsSubsystem_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Add your code here.
}

FOnlinePresenceAccelBytePtr UPresenceEssentialsSubsystem_Starter::GetPresenceInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlinePresenceAccelByte>(Subsystem->GetPresenceInterface());
}

TSharedPtr<FOnlineFriendsAccelByte, ESPMode::ThreadSafe> UPresenceEssentialsSubsystem_Starter::GetFriendsInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineFriendsAccelByte>(Subsystem->GetFriendsInterface());
}

TSharedPtr<FOnlineIdentityAccelByte, ESPMode::ThreadSafe> UPresenceEssentialsSubsystem_Starter::GetIdentityInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineIdentityAccelByte>(Subsystem->GetIdentityInterface());
}

TSharedPtr<FOnlineSessionV2AccelByte, ESPMode::ThreadSafe> UPresenceEssentialsSubsystem_Starter::GetSessionInterface() const
{
	const IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (!ensure(Subsystem))
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("The online subsystem is invalid. Please make sure OnlineSubsystemAccelByte is enabled and the DefaultPlatformService under [OnlineSubsystem] in the Engine.ini file is set to AccelByte."));
		return nullptr;
	}

	return StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Subsystem->GetSessionInterface());
}

UAccelByteWarsOnlineSessionBase* UPresenceEssentialsSubsystem_Starter::GetOnlineSession() const
{
	if (!GetGameInstance()) 
	{
		return nullptr;
	}

	return Cast<UAccelByteWarsOnlineSessionBase>(GetGameInstance()->GetOnlineSession());
}

FUniqueNetIdPtr UPresenceEssentialsSubsystem_Starter::GetPrimaryPlayerUserId()
{
	if (!GetGameInstance())
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to get current logged in player's User Id. GameInstance is invalid."));
		return nullptr;
	}

	const APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PC)
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to get current logged in player's User Id. PlayerController is invalid."));
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG_PRESENCEESSENTIALS(Warning, TEXT("Unable to get current logged in player's User Id. LocalPlayer is invalid."));
		return nullptr;
	}

	return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
}

#pragma region Module Presence Essentials Function Definitions
// TODO: Add your function definitions here.
#pragma endregion
