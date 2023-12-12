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

class UPromptSubsystem;

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

	FOnSendChatComplete* GetOnSendPrivateChatCompleteDelegates()
	{
		return &OnSendPrivateChatCompleteDelegates;
	}

	FOnChatPrivateMessageReceived* GetOnPrivateChatMessageReceivedDelegates()
	{
		return &OnPrivateChatMessageReceivedDelegates;
	}

protected:
	void OnSendPrivateChatComplete(FString UserId, FString MsgBody, FString RoomId, bool bWasSuccessful);
	void OnPrivateChatMessageReceived(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message);

	void PushPrivateChatMessageReceivedNotification(const FUniqueNetId& UserId, const TSharedRef<FChatMessage>& Message);

	FOnlineChatAccelBytePtr GetChatInterface() const;

	UPromptSubsystem* GetPromptSubsystem() const;

private:
	FOnSendChatComplete OnSendPrivateChatCompleteDelegates;
	FOnChatPrivateMessageReceived OnPrivateChatMessageReceivedDelegates;
};
