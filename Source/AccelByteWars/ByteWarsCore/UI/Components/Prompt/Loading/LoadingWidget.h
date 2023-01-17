// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "ByteWarsCore/UI/AccelByteWarsActivatableWidget.h"
#include "LoadingWidget.generated.h"

class UTextBlock;

UCLASS()
class ACCELBYTEWARS_API ULoadingWidget : public UAccelByteWarsActivatableWidget
{
	GENERATED_BODY()
	
public:
	void SetLoadingMessage(const FText& LoadingMessage);

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_LoadingMessage;
};