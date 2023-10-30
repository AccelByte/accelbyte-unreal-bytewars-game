// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WalletBalanceWidgetEntry.generated.h"

class UTextBlock;

UCLASS(Abstract)
class ACCELBYTEWARS_API UWalletBalanceWidgetEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(const FText& Balance, const FText& CurrencyCode) const;

private:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_Balance;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	UTextBlock* Tb_CurrencyCode;
};
