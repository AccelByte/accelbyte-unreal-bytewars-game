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

public:
	FOnMatchmakingStarted OnMatchmakingStartedDelegates;
	FOnMatchmakingStopped OnMatchmakingStoppedDelegates;
	FOnMatchmakingError OnMatchmakingErrorDelegates;
	FOnMatchmakingMessageReceived OnMatchmakingMessageReceivedDelegates;

	void StartMatchmaking();
	void StopMatchmaking() const;

private:
	TSharedPtr<IWebSocket> WebSocket;

	void CleanupWebSocket() const;
	void SetupWebSocket();
	bool IsIpv4(const FString& Message) const;

	UFUNCTION()
	void OnConnected() const;

	UFUNCTION()
	void OnClosed(int32 StatusCode, const FString& Reason, bool WasClean) const;

	UFUNCTION()
	void OnMessage(const FString& Message) const;

	UFUNCTION()
	void OnError(const FString& Error) const;
};
