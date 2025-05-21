// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.


#include "MultiplayerDSEssentialsSubsystemAMS_Starter.h"

#include "MultiplayerDSEssentialsLog.h"
#include "MultiplayerDSEssentialsModels.h"
#include "OnlineSubsystemUtils.h"
#include "Core/System/AccelByteWarsGameSession.h"
#include "Core/GameModes/AccelByteWarsGameMode.h"

void UMultiplayerDSEssentialsSubsystemAMS_Starter::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ABSessionInt = StaticCastSharedPtr<FOnlineSessionV2AccelByte>(Online::GetSessionInterface(GetWorld()));
	ensure(ABSessionInt);

	// TODO: Bind delegates
}

void UMultiplayerDSEssentialsSubsystemAMS_Starter::Deinitialize()
{
	Super::Deinitialize();

	// TODO: Unbind delegates
}

TSharedPtr<FAccelByteModelsV2GameSession> UMultiplayerDSEssentialsSubsystemAMS_Starter::GetSessionInfo(const FName SessionName) const
{
	const FNamedOnlineSession* Session = ABSessionInt->GetNamedSession(SessionName);
	if (!Session)
	{
		return nullptr;
	}

	const TSharedPtr<FOnlineSessionInfoAccelByteV2> SessionInfo = StaticCastSharedPtr<FOnlineSessionInfoAccelByteV2>(Session->SessionInfo);
	if (!SessionInfo)
	{
		return nullptr;
	}

	return SessionInfo->GetBackendSessionDataAsGameSession();
}

#pragma region "Tutorial module Multiplayer DS Essentials AMS"
// TODO: Define your functions here
#pragma endregion 
