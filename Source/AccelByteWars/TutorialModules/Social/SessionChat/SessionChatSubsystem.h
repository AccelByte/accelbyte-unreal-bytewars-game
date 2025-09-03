// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "SessionChatLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "SessionChatSubsystem.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API USessionChatSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
// @@@SNIPSTART SessionChatSubsystem.h-public
// @@@MULTISNIP SendChatMessage {"selectedLines": ["1", "11"]}
// @@@MULTISNIP GetOnSendChatCompleteDelegates {"selectedLines": ["1", "15-18"]}
// @@@MULTISNIP GetOnChatRoomMessageReceivedDelegates {"selectedLines": ["1", "20-23"]}
// @@@MULTISNIP GetLastChatMessages {"selectedLines": ["1", "13"]}
// @@@MULTISNIP Utilities {"selectedLines": ["1", "5-9"]}
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

	EAccelByteChatRoomType GetChatRoomType(const FString& RoomId);
	FString GetChatRoomIdBasedOnType(const EAccelByteChatRoomType ChatRoomType);
	FString GetGameSessionChatRoomId();
	FString GetPartyChatRoomId();
	bool IsMessageFromLocalUser(const FUniqueNetIdPtr UserId, const FChatMessage& Message);

	void SendChatMessage(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const FString& Message);

	bool GetLastChatMessages(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages);

	FOnSendChatComplete* GetOnSendChatCompleteDelegates()
	{
		return &OnSendChatCompleteDelegates;
	}

	FOnChatRoomMessageReceived* GetOnChatRoomMessageReceivedDelegates()
	{
		return &OnChatRoomMessageReceivedDelegates;
	}
// @@@SNIPEND

// @@@SNIPSTART SessionChatSubsystem.h-protected
// @@@MULTISNIP GetChatInterface {"selectedLines": ["1", "9"]}
// @@@MULTISNIP GetSessionInterface {"selectedLines": ["1", "10"]}
// @@@MULTISNIP GetIdentityInterface {"selectedLines": ["1", "11"]}
// @@@MULTISNIP GetPromptSubsystem {"selectedLines": ["1", "13"]}
// @@@MULTISNIP OnSendChatComplete {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnChatRoomMessageReceived {"selectedLines": ["1", "3"]}
// @@@MULTISNIP PushChatRoomMessageReceivedNotification {"selectedLines": ["1", "5"]}
// @@@MULTISNIP ReconnectChat {"selectedLines": ["1", "7"]}
protected:
	void OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnChatRoomMessageReceived(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	void PushChatRoomMessageReceivedNotification(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	void ReconnectChat(FString Message);

	FOnlineChatAccelBytePtr GetChatInterface() const;
	FOnlineSessionV2AccelBytePtr GetSessionInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubsystem() const;
// @@@SNIPEND

// @@@SNIPSTART SessionChatSubsystem.h-private
// @@@MULTISNIP OnSendChatCompleteDelegates {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnChatRoomMessageReceivedDelegates {"selectedLines": ["1", "3"]}
// @@@MULTISNIP ReconnectChatNumTries {"selectedLines": ["1", "5-6"]}
private:
	FOnSendChatComplete OnSendChatCompleteDelegates;
	FOnChatRoomMessageReceived OnChatRoomMessageReceivedDelegates;

	int32 ReconnectChatNumTries = 0;
	const int32 ReconnectChatMaxTries = 3;
// @@@SNIPEND
};
