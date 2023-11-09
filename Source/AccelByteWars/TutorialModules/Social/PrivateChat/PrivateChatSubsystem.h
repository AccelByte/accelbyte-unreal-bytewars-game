// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "PrivateChatLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "PrivateChatSubsystem.generated.h"

UCLASS()
class ACCELBYTEWARS_API UPrivateChatSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

	FString GetPrivateChatRoomId(const FUniqueNetIdPtr SenderUserId, const FUniqueNetIdPtr RecipientUserId);

	void SendPrivateChatMessage(const FUniqueNetIdPtr UserId, const FUniqueNetIdPtr RecipientUserId, const FString& Message);

	bool GetLastPrivateChatMessages(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages);
	bool IsMessageFromLocalUser(const FUniqueNetIdPtr UserId, const FChatMessage& Message);

	FOnTopicAdded* GetOnTopicAddedDelegates()
	{
		return &OnTopicAddedDelegates;
	}

	FOnTopicRemoved* GetOnTopicRemovedDelegates()
	{
		return &OnTopicRemovedDelegates;
	}

	FOnSendChatComplete* GetOnSendPrivateChatCompleteDelegates()
	{
		return &OnSendPrivateChatCompleteDelegates;
	}

	FOnChatPrivateMessageReceived* GetOnPrivateChatMessageReceivedDelegates()
	{
		return &OnPrivateChatMessageReceivedDelegates;
	}

protected:
	void OnTopicAdded(FString ChatTopicName, FString TopicId, FString UserId);
	void OnTopicRemoved(FString ChatTopicName, FString TopicId, FString SenderId);

	void OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnPrivateChatMessageReceived(const FUniqueNetId& Sender, const TSharedRef<FChatMessage>& Message);

	FOnlineChatAccelBytePtr GetChatInterface();

private:
	FOnTopicAdded OnTopicAddedDelegates;
	FOnTopicRemoved OnTopicRemovedDelegates;

	FOnSendChatComplete OnSendPrivateChatCompleteDelegates;
	FOnChatPrivateMessageReceived OnPrivateChatMessageReceivedDelegates;
};
