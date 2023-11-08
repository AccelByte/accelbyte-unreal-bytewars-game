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

#define ACCELBYTEWARS_LOCTEXT_NAMESPACE "AccelByteWars"

#define CHAT_LOCAL_SENDER_DEFAULT_USERNAME NSLOCTEXT(ACCELBYTEWARS_LOCTEXT_NAMESPACE, "Chat Local Sender Default Username", "You")