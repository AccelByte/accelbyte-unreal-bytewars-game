// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#include "ChatWidgetEntry.h"

#include "Social/ChatEssentials/ChatEssentialsLog.h"
#include "Social/ChatEssentials/ChatEssentialsModels.h"

#include "Components/TextBlock.h"

void UChatWidgetEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Super::NativeOnListItemObjectSet(ListItemObject);

	const UChatData* ChatData = Cast<UChatData>(ListItemObject);
	if (!ChatData)
	{
		UE_LOG_CHATESSENTIALS(Warning, TEXT("Cannot handle data for chat widget entry. Chat data is not valid."));
		return;
	}

	Tb_Sender->SetText(ChatData->bIsSenderLocal ? CHAT_LOCAL_SENDER_DEFAULT_USERNAME : FText::FromString(ChatData->Sender));
	Tb_Message->SetText(FText::FromString(ChatData->Message));

	SetChatSenderType(ChatData->bIsSenderLocal);
}
