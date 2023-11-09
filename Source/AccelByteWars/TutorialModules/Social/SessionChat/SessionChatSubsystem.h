// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineSessionInterfaceV2AccelByte.h"
#include "SessionChatLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "SessionChatSubsystem.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API USessionChatSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

	FString GetChatRoomIdBasedOnType(const EAccelByteChatRoomType ChatRoomType);
	FString GetGameSessionChatRoomId();
	FString GetPartyChatRoomId();

	void SendChatMessage(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const FString& Message);

	bool GetLastChatMessages(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages);
	bool IsMessageFromLocalUser(const FUniqueNetIdPtr UserId, const FChatMessage& Message);

	EAccelByteChatRoomType GetChatRoomType(const FString& RoomId);

	FOnTopicAdded* GetOnTopicAddedDelegates()
	{
		return &OnTopicAddedDelegates;
	}

	FOnTopicRemoved* GetOnTopicRemovedDelegates()
	{
		return &OnTopicRemovedDelegates;
	}

	FOnSendChatComplete* GetOnSendChatCompleteDelegates()
	{
		return &OnSendChatCompleteDelegates;
	}

	FOnChatRoomMessageReceived* GetOnChatRoomMessageReceivedDelegates()
	{
		return &OnChatRoomMessageReceivedDelegates;
	}

protected:
	void OnTopicAdded(FString ChatTopicName, FString TopicId, FString UserId);
	void OnTopicRemoved(FString ChatTopicName, FString TopicId, FString SenderId);

	void OnSendChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnChatRoomMessageReceived(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	void PushChatRoomMessageReceivedNotification(const FUniqueNetId& Sender, const FChatRoomId& RoomId, const TSharedRef<FChatMessage>& Message);

	FOnlineChatAccelBytePtr GetChatInterface();
	FOnlineSessionV2AccelBytePtr GetSessionInterface();

	UPromptSubsystem* GetPromptSubystem();

private:
	FOnTopicAdded OnTopicAddedDelegates;
	FOnTopicRemoved OnTopicRemovedDelegates;

	FOnSendChatComplete OnSendChatCompleteDelegates;
	FOnChatRoomMessageReceived OnChatRoomMessageReceivedDelegates;
};
