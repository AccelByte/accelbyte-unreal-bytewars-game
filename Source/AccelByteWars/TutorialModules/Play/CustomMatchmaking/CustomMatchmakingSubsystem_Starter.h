// Copyright (c) 2025 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CustomMatchmakingModels.h"
#include "CustomMatchmakingSubsystem_Starter.generated.h"

UCLASS()
class ACCELBYTEWARS_API UCustomMatchmakingSubsystem_Starter : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

public:
	FOnMatchmakingStarted OnMatchmakingStartedDelegates;
	FOnMatchmakingStopped OnMatchmakingStoppedDelegates;
	FOnMatchmakingError OnMatchmakingErrorDelegates;
	FOnMatchmakingMessageReceived OnMatchmakingMessageReceivedDelegates;

#pragma region "Tutorial"
	// Declare your public functions here
#pragma endregion

private:
	TSharedPtr<IWebSocket> WebSocket;

	FString PendingDisconnectReason = TEXT("");

	void CleanupWebSocket();
	void SetupWebSocket();

	void ThrowInvalidPayloadError();

#pragma region "Tutorial"
	// Declare your private functions here
#pragma endregion
};
