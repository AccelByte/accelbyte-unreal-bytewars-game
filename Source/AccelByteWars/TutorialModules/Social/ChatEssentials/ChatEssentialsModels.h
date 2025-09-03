// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ChatEssentialsModels.generated.h"

UCLASS()
class ACCELBYTEWARS_API UChatData : public UObject
{
	GENERATED_BODY()

public:
	FString Sender;
	FString Message;
	bool bIsSenderLocal;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FChatOnSubmit, const FText&);

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

#define CHAT_LOCAL_SENDER_DEFAULT_USERNAME NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Chat Local Sender Default Username", "You")
#define SEND_CHAT_FAILED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Send Chat Failed Message", "Failed to send chat message")
#define PRIVATE_CHAT_RECEIVED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Private Chat Received Message", "Incoming private chat from {0}")
#define GAMESESSION_CHAT_RECEIVED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Game Session Chat Received Message", "Incoming game session chat from {0}")
#define PARTY_CHAT_RECEIVED_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Party Chat Received Message", "Incoming party chat from {0}")
#define NO_CHAT_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "No Chat Message", "No messages")
#define INVALID_CHAT_ROOM_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Invalid Chat Room Message", "Chat is not available")
#define INVALID_GAMESESSION_CHAT_ROOM_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Invalid Session Chat Room Message", "You are not in a game session")
#define INVALID_PARTY_CHAT_ROOM_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Invalid Party Chat Room Message", "You are not in a party")
#define FAILED_TO_LOAD_CHAT_MESSAGE NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Failed to Load Chat Messages", "Failed to load chat messages")