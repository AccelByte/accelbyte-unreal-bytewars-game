// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Core/UI/Components/AccelByteWarsWidgetEntry.h"
#include "ChatWidgetEntry.generated.h"

class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UChatWidgetEntry : public UAccelByteWarsWidgetEntry
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetChatSenderType(const bool bIsLocalSender);

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Sender;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Message;
};
