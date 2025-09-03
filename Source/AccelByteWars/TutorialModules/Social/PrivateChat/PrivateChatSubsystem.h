// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSubsystemAccelByte.h"
#include "OnlineChatInterfaceAccelByte.h"
#include "OnlineIdentityInterfaceAccelByte.h"
#include "PrivateChatLog.h"
#include "Core/AssetManager/TutorialModules/TutorialModuleSubsystem.h"
#include "PrivateChatSubsystem.generated.h"

class UPromptSubsystem;

UCLASS()
class ACCELBYTEWARS_API UPrivateChatSubsystem : public UTutorialModuleSubsystem
{
	GENERATED_BODY()

// @@@SNIPSTART PrivateChatSubsystem.h-public
// @@@MULTISNIP SendPrivateChatMessage {"selectedLines": ["1", "8"]}
// @@@MULTISNIP GetOnSendPrivateChatCompleteDelegates {"selectedLines": ["1", "12-15"]}
// @@@MULTISNIP GetOnPrivateChatMessageReceivedDelegates {"selectedLines": ["1", "17-20"]}
// @@@MULTISNIP GetLastPrivateChatMessages {"selectedLines": ["1", "10"]}
// @@@MULTISNIP Utilities {"selectedLines": ["1", "5-6"]}
public:
	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize();

	FString GetPrivateChatRoomId(const FUniqueNetIdPtr SenderUserId, const FUniqueNetIdPtr RecipientUserId);
	bool IsMessageFromLocalUser(const FUniqueNetIdPtr UserId, const FChatMessage& Message);

	void SendPrivateChatMessage(const FUniqueNetIdPtr UserId, const FUniqueNetIdPtr RecipientUserId, const FString& Message);

	bool GetLastPrivateChatMessages(const FUniqueNetIdPtr UserId, const FChatRoomId& RoomId, const int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages);

	FOnSendChatComplete* GetOnSendPrivateChatCompleteDelegates()
	{
		return &OnSendPrivateChatCompleteDelegates;
	}

	FOnChatPrivateMessageReceived* GetOnPrivateChatMessageReceivedDelegates()
	{
		return &OnPrivateChatMessageReceivedDelegates;
	}
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.h-protected
// @@@MULTISNIP GetChatInterface {"selectedLines": ["1", "9"]}
// @@@MULTISNIP GetIdentityInterface {"selectedLines": ["1", "10"]}
// @@@MULTISNIP GetPromptSubsystem {"selectedLines": ["1", "12"]}
// @@@MULTISNIP OnSendPrivateChatComplete {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnPrivateChatMessageReceived {"selectedLines": ["1", "3"]}
// @@@MULTISNIP PushPrivateChatMessageReceivedNotification {"selectedLines": ["1", "5"]}
// @@@MULTISNIP ReconnectChat {"selectedLines": ["1", "7"]}
protected:
	void OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnPrivateChatMessageReceived(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message);

	void PushPrivateChatMessageReceivedNotification(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message);

	void ReconnectChat(FString Message);

	FOnlineChatAccelBytePtr GetChatInterface() const;
	FOnlineIdentityAccelBytePtr GetIdentityInterface() const;

	UPromptSubsystem* GetPromptSubsystem() const;
// @@@SNIPEND

// @@@SNIPSTART PrivateChatSubsystem.h-private
// @@@MULTISNIP OnSendPrivateChatCompleteDelegates {"selectedLines": ["1-2"]}
// @@@MULTISNIP OnPrivateChatMessageReceivedDelegates {"selectedLines": ["1", "3"]}
// @@@MULTISNIP ReconnectChatNumTries {"selectedLines": ["1", "5-6"]}
private:
	FOnSendChatComplete OnSendPrivateChatCompleteDelegates;
	FOnChatPrivateMessageReceived OnPrivateChatMessageReceivedDelegates;

	int32 ReconnectChatNumTries = 0;
	const int32 ReconnectChatMaxTries = 3;
// @@@SNIPEND
};
