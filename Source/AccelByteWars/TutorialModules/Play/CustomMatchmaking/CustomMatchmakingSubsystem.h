// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "CustomMatchmakingModels.h"
#include "CustomMatchmakingSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UCustomMatchmakingSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

// @@@SNIPSTART CustomMatchmakingSubsystem.h-public
// @@@MULTISNIP Delegates {"selectedLines": ["1-5"]}
// @@@MULTISNIP Functions {"selectedLines": ["1", "7-8"]}
public:
	FOnMatchmakingStarted OnMatchmakingStartedDelegates;
	FOnMatchmakingStopped OnMatchmakingStoppedDelegates;
	FOnMatchmakingError OnMatchmakingErrorDelegates;
	FOnMatchmakingMessageReceived OnMatchmakingMessageReceivedDelegates;

	void StartMatchmaking();
	void StopMatchmaking();
// @@@SNIPEND

// @@@SNIPSTART CustomMatchmakingSubsystem.h-private
// @@@MULTISNIP Websocket {"selectedLines": ["1-2"]}
// @@@MULTISNIP Setup {"selectedLines": ["1", "7"]}
// @@@MULTISNIP PendingDisconnectReason {"selectedLines": ["1", "4"]}
// @@@MULTISNIP Cleanup {"selectedLines": ["1", "6"]}
// @@@MULTISNIP Functions {"selectedLines": ["1", "9-19"]}
// @@@MULTISNIP ThrowInvalidPayloadError {"selectedLines": ["1", "21"]}
private:
	TSharedPtr<IWebSocket> WebSocket;

	FString PendingDisconnectReason = TEXT("");

	void CleanupWebSocket();
	void SetupWebSocket();

	UFUNCTION()
	void OnConnected() const;

	UFUNCTION()
	void OnClosed(int32 StatusCode, const FString& Reason, bool WasClean);

	UFUNCTION()
	void OnMessage(const FString& Message);

	UFUNCTION()
	void OnError(const FString& Error) const;

	void ThrowInvalidPayloadError();
// @@@SNIPEND
};
